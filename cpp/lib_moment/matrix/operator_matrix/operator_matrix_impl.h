/**
 * operator_matrix_impl.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "operator_matrix.h"
#include "operator_matrix_factory.h"

#include "dictionary/dictionary.h"
#include "scenarios/context.h"

#include "multithreading/multithreading.h"

#include <concepts>
#include <complex>

namespace Moment {

    template<typename t> struct OSGIndexType;
    template<> struct OSGIndexType<size_t> {
        using type = size_t;
    };

    [[nodiscard]] constexpr size_t get_osg_index(const size_t index) {
        return index;
    }

    /**
     * Concept: functor that can generate matrices.
     */
    template<typename functor_t, typename index_t>
    concept generates_operator_matrices = requires(const functor_t& functor_cref,
                                                   const OperatorSequence& op_seq_cref,
                                                   const index_t& an_index) {
        {functor_cref(op_seq_cref, op_seq_cref)} -> std::convertible_to<OperatorSequence>;
        {functor_t::determine_prefactor(an_index)} -> std::convertible_to<std::complex<double>>;
        {functor_t::should_be_hermitian(an_index)} -> std::same_as<bool>;
    };

    template<typename index_t, generates_operator_matrices<index_t> functor_t, typename matrix_t>
    class OperatorMatrixImpl : public OperatorMatrix {
    public:
        using IndexT = index_t;
        using OSGIndexT = typename OSGIndexType<index_t>::type;

        /** Generator functor wrapped with call to contextual alias resolution. */
        struct aliasing_functor_t {
        public:
            const Context& context;
            const functor_t underlying_functor;

            inline aliasing_functor_t(const Context& context, functor_t functor)
                : context{context}, underlying_functor{std::move(functor)} { }

            [[nodiscard]] inline OperatorSequence operator()(const OperatorSequence& lhs,
                                                             const OperatorSequence& rhs) const {
                return context.simplify_as_moment(underlying_functor(lhs, rhs));
            }
        };

        const IndexT Index;

        OperatorMatrixImpl(const Context& context, IndexT input_index,
                           std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat) :
            OperatorMatrix{context, std::move(op_seq_mat)}, Index{std::move(input_index)} {

        }

        OperatorMatrixImpl(const OperatorMatrixImpl& rhs) = delete;

        OperatorMatrixImpl(OperatorMatrixImpl&& rhs) = default;

        /**
         * Get pair of operator sequence generators associated with this matrix.
         */
        [[nodiscard]] const OSGPair& generators() const override {
            const auto& dictionary = this->context.dictionary();
            return dictionary(get_osg_index(this->Index));
        }

        /**
         * Returns underlying operator matrix pointer, or a nullptr if matrix is not of a matching type
         */
        [[nodiscard]] static matrix_t const * to_operator_matrix_ptr(const SymbolicMatrix& matrix) noexcept {
            if (!matrix.has_operator_matrix()) {
                return nullptr;
            }
            return dynamic_cast<matrix_t const *>(&(matrix.operator_matrix()));
        }

        /**
         * Full creation stack, with generation, symbol-registry and multithreading.
         */
        [[nodiscard]] static std::unique_ptr<MonomialMatrix>
        create_matrix(const Context& context, SymbolTable& symbols, IndexT index,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional) {

            const bool should_be_hermitian = functor_t::should_be_hermitian(index);
            const std::complex<double> prefactor = functor_t::determine_prefactor(index);

            if (context.can_have_aliases()) {
                OperatorMatrixFactory<matrix_t, class Context, OSGIndexT, aliasing_functor_t>
                        creation_factory{context, symbols, get_osg_index(index),
                                         aliasing_functor_t{context, functor_t{context, index}},
                                         should_be_hermitian, prefactor, mt_policy};
                return creation_factory.execute(index);
            } else {
                OperatorMatrixFactory<matrix_t, class Context, OSGIndexT, functor_t>
                        creation_factory{context, symbols, get_osg_index(index), functor_t{context, index},
                                         should_be_hermitian, prefactor, mt_policy};

                return creation_factory.execute(index);
            }
        }
    };
}