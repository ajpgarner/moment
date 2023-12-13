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

    /**
     * Concept: type that defines generation of matrix, acting as a functor for each element within.
     */
    template<typename functor_t, typename index_t, typename context_t>
    concept generates_operator_matrices = requires(const functor_t& functor_cref,
                                                   const OperatorSequence& op_seq_cref,
                                                   const index_t& an_index,
                                                   const context_t& context,
                                                   typename functor_t::OSGIndex an_osg_index) {
        {functor_t{context, an_index}};
        {functor_cref(op_seq_cref, op_seq_cref)} -> std::convertible_to<OperatorSequence>;
        {functor_t::determine_prefactor(an_index)} -> std::convertible_to<std::complex<double>>;
        {functor_t::should_be_hermitian(an_index)} -> std::same_as<bool>;
        {functor_t::get_osg_index(an_index)} -> std::same_as<typename functor_t::OSGIndex>;
        {functor_t::get_generators(context, an_osg_index)} -> std::same_as<const OSGPair&>;
    };

    template<typename index_t, typename context_t, generates_operator_matrices<index_t, context_t> functor_t,
            typename matrix_t>
    class OperatorMatrixImpl : public OperatorMatrix {
    public:
        /** Operator index type */
        using IndexT = index_t;

        /** Operator sequence generator index type */
        using OSGIndexT = typename functor_t::OSGIndex;

        /** Generator functor wrapped with call to contextual alias resolution. */
        struct aliasing_functor_t {
        public:
            const context_t& context;
            const functor_t underlying_functor;

            inline aliasing_functor_t(const context_t& context, functor_t functor)
                : context{context}, underlying_functor{std::move(functor)} { }

            [[nodiscard]] inline OperatorSequence operator()(const OperatorSequence& lhs,
                                                             const OperatorSequence& rhs) const {
                return context.simplify_as_moment(underlying_functor(lhs, rhs));
            }

            [[nodiscard]] static inline OSGIndexT get_osg_index(const IndexT& index) {
                return functor_t::get_osg_index(index);
            }

            [[nodiscard]] static inline const OSGPair&
            get_generators(const context_t& context, const OSGIndexT& osg_index) {
                return functor_t::get_generators(context, osg_index);
            }

        };

        /**
         * The index object that labels this operator matrix.
         */
        const IndexT Index;

        /**
         * Specialized context.
         */
        const context_t& SpecializedContext;

        /**
         *
         * @param context The (possibly specialized) operator context
         * @param input_index The index labelling the matrix
         * @param op_seq_mat The generated matrix.
         */
        OperatorMatrixImpl(const context_t& context, IndexT input_index,
                           std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat) :
            OperatorMatrix{context, std::move(op_seq_mat)}, Index{std::move(input_index)}, SpecializedContext{context} {
        }

        OperatorMatrixImpl(const OperatorMatrixImpl& rhs) = delete;

        OperatorMatrixImpl(OperatorMatrixImpl&& rhs) = default;


        /**
         * The part of the index that labels the OSGs associated with this operator matrix.
         */
        [[nodiscard]] inline const OSGIndexT OSGIndex() const noexcept {
            return functor_t::get_osg_index(Index);
        }

        /**
         * Get pair of operator sequence generators associated with this matrix.
         */
        [[nodiscard]] inline const OSGPair& generators() const final {
            return functor_t::get_generators(this->SpecializedContext, functor_t::get_osg_index(this->Index));
        }

        /**
         * Returns underlying operator matrix pointer, or a nullptr if matrix is not of a matching type.
         */
        [[nodiscard]] static matrix_t const * to_operator_matrix_ptr(const SymbolicMatrix& matrix) noexcept {
            if (!matrix.has_operator_matrix()) {
                return nullptr;
            }
            return dynamic_cast<matrix_t const *>(&(matrix.operator_matrix()));
        }

        /**
         * Names matrix by its index name.
         */
        [[nodiscard]] std::string description() const override {
            return this->Index.to_string();
        }

        /**
         * Full creation stack, with generation, symbol-registry and multithreading.
         */
        [[nodiscard]] static std::unique_ptr<MonomialMatrix>
        create_matrix(const context_t& context, SymbolTable& symbols, IndexT index,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional) {

            const bool should_be_hermitian = functor_t::should_be_hermitian(index);
            const std::complex<double> prefactor = functor_t::determine_prefactor(index);

            if (context.can_have_aliases()) {
                OperatorMatrixFactory<matrix_t, context_t, OSGIndexT, aliasing_functor_t>
                        creation_factory{context, symbols, functor_t::get_osg_index(index),
                                         aliasing_functor_t{context, functor_t{context, index}},
                                         should_be_hermitian, prefactor, mt_policy};
                return creation_factory.execute(index);
            } else {
                OperatorMatrixFactory<matrix_t, context_t, OSGIndexT, functor_t>
                        creation_factory{context, symbols, functor_t::get_osg_index(index),
                                         functor_t{context, index},
                                         should_be_hermitian, prefactor, mt_policy};

                return creation_factory.execute(index);
            }
        }
    };
}