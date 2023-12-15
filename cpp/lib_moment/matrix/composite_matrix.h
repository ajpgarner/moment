/**
 * composite_matrix.h
 *
 * Forms polynomial matrices by summing together a collection of other matrices. This is the base class of, e.g.
 * polynomial localizing matrices of various flavours, and polynomial (anti-)commutator matrices.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "dictionary/raw_polynomial.h"

#include "matrix_system/matrix_indices.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

#include "symbolic/polynomial_factory.h"

#include <cassert>

#include <complex>
#include <concepts>
#include <memory>
#include <optional>
#include <vector>

namespace Moment {
    class MatrixSystem;

    /**
     * A polynomial matrix formed by summing together a collection of other matrices.
     */
    class CompositeMatrix : public PolynomialMatrix {
    public:
        /**
         * Construction information: defines the constituents that form a composite matrix.
         */
        struct ConstituentInfo {
        public:
            /** The size of the matrix */
            size_t matrix_dimension;

            /** Pointers to the elements of the matrix */
            std::vector<std::pair<SymbolicMatrix const *, std::complex<double>>> elements;

            /** Delete copy constructor. */
            ConstituentInfo(const ConstituentInfo& rhs) = delete;

            /** Default move constructor. */
            ConstituentInfo(ConstituentInfo&& rhs) = default;

            /** Construct empty collection. */
            explicit ConstituentInfo(size_t dim = 0) : matrix_dimension{dim} { }

            /** Construct 'collection' of one single matrix. */
            explicit ConstituentInfo(const SymbolicMatrix& input, std::complex<double> scale = {1.0, 0.0})
                : matrix_dimension{input.Dimension()} {
                this->elements.emplace_back(&input, scale);
            }

            /** Do not construct collection including pointer to transient r-value. */
            explicit ConstituentInfo(SymbolicMatrix&& input) = delete;

            /** Size of constituents is number of elements. */
            [[nodiscard]] inline size_t size() const noexcept {
                return this->elements.size();
            }

            /** Constituents are an empty collection if it has no elements. */
            [[nodiscard]] inline bool empty() const noexcept {
                return this->elements.empty();
            }

            /** Begin iteration over constituents */
            [[nodiscard]] auto begin() const noexcept {
                return this->elements.cbegin();
            }

            /** End iteration over constituents */
            [[nodiscard]] auto end() const noexcept {
                return this->elements.cend();
            }

            /** Get constituent by index. */
            [[nodiscard]] std::pair<SymbolicMatrix const *, std::complex<double>>
            operator[](size_t index) const noexcept {
                assert(index < this->elements.size());
                return this->elements[index];
            }

            /** Attempt to set dimension automatically; returns false if could not */
            bool auto_set_dimension() noexcept {
                // Cannot infer dimension if no matrices added
                if (this->elements.empty()) {
                    return false;
                }

                // Otherwise, first element sets dimension
                assert(this->elements.front().first);
                this->matrix_dimension = this->elements.front().first->Dimension();
                return true;
            }

        };

    protected:
        ConstituentInfo constituents;

    public:
        /** Constructor for non-empty polynomial localizing matrix. */
        CompositeMatrix(const Context& context, SymbolTable& symbols,
                        const PolynomialFactory& factory, ConstituentInfo&& constituents);

        /** Get constituent part information */
        [[nodiscard]] const ConstituentInfo& Constituents() const noexcept {
            return this->constituents;
        }


    protected:
        /**
         * Synthesize constituent data into a single polynomial matrix data object.
         */
        [[nodiscard]] static std::unique_ptr<PolynomialMatrix::MatrixData>
        compile_to_polynomial_matrix_data(const PolynomialFactory& factory,
                                          const ConstituentInfo& constituents);
    };

    /**
     * Generic implementation of composition of monomial matrix of a particular type into its polynomial equivalent.
     *
     * @tparam matrix_system_t The matrix system type
     * @tparam polynomial_index_t The polynomial index type.
     * @tparam monomial_indices_t The index storage bank type, that produces the constituent monomial matrices.
     * @tparam realized_type
     */
    template<typename matrix_system_t,
             typename polynomial_index_t,
             typename monomial_indices_t>
    class CompositeMatrixImpl : public CompositeMatrix {
    public:
        using ImplType = CompositeMatrixImpl<matrix_system_t, polynomial_index_t, monomial_indices_t>;
        using PolynomialIndex = polynomial_index_t;
        using MonomialIndex = typename polynomial_index_t::ComponentIndex;
        using OSGIndex = typename polynomial_index_t::OSGIndex;


        /** Full index that defines this polynomial matrix */
        const PolynomialIndex index;

        const std::optional<RawPolynomial> unaliased_index;

    public:
        CompositeMatrixImpl(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                            PolynomialIndex index_in, CompositeMatrix::ConstituentInfo&& constituents_in,
                            std::optional<RawPolynomial> unaliased_index_in = std::nullopt)
                : CompositeMatrix{context, symbols, factory, std::move(constituents_in)},
                  index{std::move(index_in)}, unaliased_index{std::move(unaliased_index_in)} {
            if (unaliased_index.has_value()) {
                this->description = PolynomialIndex::raw_to_string(context, symbols,
                                                                   index_in.Level, unaliased_index.value());
            } else {
                this->description = index.to_string(context, symbols);
            }
        }

        /**
        * Constructs a polynomial matrix from a Polynomial, invoking the construction of any necessary components.
        * @param write_lock A locked write lock for the system.
        * @param system The matrix system.
        * @param monomial_matrices The indexing object for the monomial matrices that will be composited.
        * @param polynomial_index The index of the polynomial matrix to construct.
        * @param mt_policy The multi-threading policy to use (potentially relevant in the generation of components).
        * @return A newly created polynomial matrix.
        */
        static std::unique_ptr<ImplType>
        create(const MaintainsMutex::WriteLock& write_lock,
               matrix_system_t& system, monomial_indices_t& monomial_matrices,
               PolynomialIndex polynomial_index,
               Multithreading::MultiThreadPolicy mt_policy) {
            assert(system.is_locked_write_lock(write_lock));

            // System parts:
            const auto& context = system.Context();
            auto& symbols = system.Symbols();
            const auto& poly_factory = system.polynomial_factory();

            // First ensure constituent parts exist
            CompositeMatrix::ConstituentInfo constituents;
            constituents.elements.reserve(polynomial_index.Polynomial.size());
            for (auto [mono_index, factor] : polynomial_index.MonomialIndices(symbols)) {
                auto [mono_offset, mono_matrix] = monomial_matrices.create(write_lock, mono_index, mt_policy);
                constituents.elements.emplace_back(&mono_matrix, factor);
            }

            // If no constituents, we have to query for matrix size by asking system about its OSG:
            if (!constituents.auto_set_dimension()) {
                constituents.matrix_dimension = system.osg_size(polynomial_index.Level);
            }

            return std::make_unique<ImplType>(context, symbols, system.polynomial_factory(),
                                              std::move(polynomial_index),
                                              std::move(constituents));
        }

        /**
         * Constructs a polynomial matrix from a RawPolynomial, invoking the construction of any necessary components.
         * @param write_lock A locked write lock for the system.
         * @param system The matrix system.
         * @param monomial_matrices The indexing object for the monomial matrices that will be composited.
         * @param osg_index The OSG index for the matrices.
         * @param raw_polynomials The raw polynomial to construct.
         * @param mt_policy The multi-threading policy to use (potentially relevant in the generation of components).
         * @return A newly created polynomial matrix.
         */
        static std::unique_ptr<ImplType>
        create_from_raw(const MaintainsMutex::WriteLock& write_lock,
                        matrix_system_t& system,
                        monomial_indices_t& monomial_matrices,
                        OSGIndex osg_index, const RawPolynomial& raw_polynomial,
                        Multithreading::MultiThreadPolicy mt_policy) {
            assert(system.is_locked_write_lock(write_lock));

            // Get system information for symbol registration
            auto& symbols = system.Symbols();
            const auto& context = system.Context();
            const auto& poly_factory = system.polynomial_factory();

            // If there are no aliases in the scenario, we can register raw polynomial into symbols to make new index,
            // then invoke the non-raw construction:
            if (!context.can_have_aliases()) {
                return ImplType::create(write_lock, system, monomial_matrices,
                                        polynomial_index_t{std::move(osg_index),
                                                           poly_factory.register_and_construct(symbols,
                                                                                               raw_polynomial)},
                                        mt_policy);
            }

            // Otherwise, we have to treat the constituents of the raw polynomial one element at a time.
            // First ensure constituent parts exist:
            CompositeMatrix::ConstituentInfo constituents;
            constituents.elements.reserve(raw_polynomial.size());
            for (const auto& [mono_sequence, factor] : raw_polynomial) {
                auto [mono_offset, mono_matrix] = monomial_matrices.create(write_lock,
                                                                           MonomialIndex{osg_index, mono_sequence},
                                                                           mt_policy);
                constituents.elements.emplace_back(&mono_matrix, factor);
            }

            // If no constituents, we have to query for matrix size by asking system about its OSG:
            if (!constituents.auto_set_dimension()) {
                constituents.matrix_dimension = system.osg_size(osg_index);
            }

            // Make approximate and true index
            auto aliased_polynomial = poly_factory.construct(raw_polynomial);
            return std::make_unique<ImplType>(context, symbols, system.polynomial_factory(),
                                              polynomial_index_t{std::move(osg_index), std::move(aliased_polynomial)},
                                              std::move(constituents),
                                              raw_polynomial);


        }
    };
}