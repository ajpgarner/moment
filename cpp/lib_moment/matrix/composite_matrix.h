/**
 * composite_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial_matrix.h"
#include "monomial_matrix.h"

#include <cassert>
#include <complex>
#include <memory>
#include <vector>

namespace Moment {
    /**
     * A polynomial matrix formed by summing together a collection of other matrices.
     */
    class CompositeMatrix : public PolynomialMatrix {
    public:
        struct ConstituentInfo {
        public:
            size_t matrix_dimension;
            std::vector<std::pair<const SymbolicMatrix*, std::complex<double>>> elements;

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

    protected:
        /**
         * Synthesize constituent data into a single polynomial matrix data object.
         */
        [[nodiscard]] static std::unique_ptr<PolynomialMatrix::MatrixData>
        compile_to_polynomial_matrix_data(const PolynomialFactory& factory,
                                          const ConstituentInfo& constituents);
    };
}