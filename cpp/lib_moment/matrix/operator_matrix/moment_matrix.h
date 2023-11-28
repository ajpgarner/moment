/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"
#include "operator_matrix_impl.h"

#include "scenarios/context.h"
#include "dictionary/dictionary.h"

namespace Moment {

    /**
     * Defines how a moment matrix is generated from its NPA hierarchy level.
     */
    struct MomentMatrixGenerator {
    public:
        using OSGIndex = size_t;

        const size_t index;

        constexpr MomentMatrixGenerator(const Context& /**/, const size_t index)
                : index{index} { }

        /**
         * Elements of moment matrix are essentially lhs * rhs.
         */
        [[nodiscard]] inline OperatorSequence
        operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
            return lhs * rhs;
        }

        /** Moment matrices are always Hermitian. */
        [[nodiscard]] inline constexpr static bool should_be_hermitian(const size_t /**/) noexcept { return true; }

        /** Moment matrices always have a prefactor of +1. */
        [[nodiscard]] inline constexpr static std::complex<double>
        determine_prefactor(const size_t /**/) noexcept { return std::complex<double>{1.0, 0.0}; }

        /** Pass-through index to get OSG index. */
        [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const size_t input) { return input; }

        /** Get normal OSGs */
        [[nodiscard]] inline static const OSGPair& get_generators(const Context& context, const size_t input) {
            return context.dictionary().Level(input);
        }
    };
    static_assert(generates_operator_matrices<MomentMatrixGenerator, size_t, Context>);

    /**
     * Full moment matrix of operators.
     */
    class MomentMatrix;
    class MomentMatrix : public OperatorMatrixImpl<size_t, Context, MomentMatrixGenerator, MomentMatrix> {
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
    };
}