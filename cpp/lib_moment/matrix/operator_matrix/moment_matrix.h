/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"
#include "operator_matrix_impl.h"

namespace Moment {

    struct MomentMatrixGenerator {
    public:
        const size_t index;

        constexpr MomentMatrixGenerator(const Context& /**/, const size_t index)
                : index{index} { }

        [[nodiscard]] inline OperatorSequence
        operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
            return lhs * rhs;
        }

        /** Moment matrices are always Hermitian. */
        [[nodiscard]] inline constexpr static bool should_be_hermitian(const size_t /**/) noexcept { return true; }

        /** Moment matrices always have a prefactor of +1. */
        [[nodiscard]] inline constexpr static std::complex<double>
        determine_prefactor(const size_t /**/) noexcept { return std::complex<double>{1.0, 0.0}; }
    };

    /**
     * Full moment matrix of operators.
     */
    class MomentMatrix;
    class MomentMatrix : public OperatorMatrixImpl<size_t, MomentMatrixGenerator, MomentMatrix> {
    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param level The hierarchy depth.
         * @param mt_policy Whether or not to use multi-threaded creation.
         */
        MomentMatrix(const Context& context, size_t level, std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
            : OperatorMatrixImpl{context, level, std::move(op_seq_mat)} { }

        /**
         * String label for this moment matrix.
         */
        [[nodiscard]] std::string description() const override;

    public:
        /**
         * If supplied input is symbol matrix associated with a monomial moment matrix, extract that moment matrix.
         * Otherwise, returns nullptr.
         */
        static const MomentMatrix* as_monomial_moment_matrix_ptr(const SymbolicMatrix& input) noexcept;
    };
}