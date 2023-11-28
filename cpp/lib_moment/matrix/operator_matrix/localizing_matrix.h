/**
 * localizing_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix_system/localizing_matrix_index.h"
#include "operator_matrix.h"
#include "operator_matrix_impl.h"

#include "scenarios/context.h"
#include "dictionary/dictionary.h"


namespace Moment {

    /**
     * Defines how a localizing matrix is generated from its NPA hierarchy level.
     */
    struct LocalizingMatrixGenerator {
    public:
        using OSGIndex = size_t;

        const LocalizingMatrixIndex& lmi;

        constexpr LocalizingMatrixGenerator(const Context& /**/, const LocalizingMatrixIndex& lmi)
            : lmi{lmi} { }

        [[nodiscard]] inline OperatorSequence
        operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
            return lhs * (lmi.Word * rhs);
        }

        /** Localizing matrices are Hermitian if their word is Hermitian. */
        [[nodiscard]] static inline bool should_be_hermitian(const LocalizingMatrixIndex& lmi) {
            return !is_imaginary(lmi.Word.get_sign()) && (lmi.Word.hash() == lmi.Word.conjugate().hash());
        }

        /** Localizing matrices always have a prefactor of +1. */
        [[nodiscard]] inline constexpr static std::complex<double>
        determine_prefactor(const LocalizingMatrixIndex& /**/) noexcept { return std::complex<double>{1.0, 0.0}; }

        /** Level of localizing matrix index is the OSG index. */
        [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const LocalizingMatrixIndex& input) {
            return input.Level;
        }

        /** Get normal OSGs */
        [[nodiscard]] inline static const OSGPair& get_generators(const Context& context, const size_t input) {
            return context.dictionary().Level(input);
        }
    };
    static_assert(generates_operator_matrices<LocalizingMatrixGenerator, LocalizingMatrixIndex, Context>);


    class LocalizingMatrix;
    class LocalizingMatrix
            : public OperatorMatrixImpl<LocalizingMatrixIndex, Context, LocalizingMatrixGenerator, LocalizingMatrix> {
    public:
        /**
          * Constructs a localizing matrix at the requested hierarchy depth (level) for the supplied context,
          * with the supplied word.
          * @param context The setting/scenario.
          * @param lmi Index, describing the hierarchy depth and localizing word.
          */
        LocalizingMatrix(const Context& context, LocalizingMatrixIndex lmi,
                         std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
             : OperatorMatrixImpl<LocalizingMatrixIndex, Context, LocalizingMatrixGenerator, LocalizingMatrix>{
                        context, std::move(lmi), std::move(op_seq_mat)} { }

        [[nodiscard]] std::string description() const override;
    };
}