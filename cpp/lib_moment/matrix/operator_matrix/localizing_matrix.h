/**
 * localizing_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix_system/localizing_matrix_index.h"
#include "operator_matrix.h"

#include "multithreading/multithreading.h"

namespace Moment {

    class LocalizingMatrix : public OperatorMatrix {
    public:
        /**
         * "Index" of this localizing matrix, containing its depth and localizing word.
         */
        const LocalizingMatrixIndex Index;

        /**
          * Constructs a localizing matrix at the requested hierarchy depth (level) for the supplied context,
          * with the supplied word.
          * @param context The setting/scenario.
          * @param lmi Index, describing the hierarchy depth and localizing word.
          */
        LocalizingMatrix(const Context& context, LocalizingMatrixIndex lmi,
                         std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

    public:

        LocalizingMatrix(LocalizingMatrix&& rhs) = default;

        ~LocalizingMatrix() noexcept;

        /**
         * The generating word for this localizing matrix.
         */
        [[nodiscard]] constexpr const OperatorSequence& Word() const noexcept {
            return Index.Word;
        }

        /**
         * The hierarchy depth of this localizing matrix.
         */
        [[nodiscard]] constexpr size_t Level() const noexcept {
            return Index.Level;
        }

        [[nodiscard]] std::string description() const override;


        /**
         * Full creation stack, with possible multithreading.
         */
        static std::unique_ptr<MonomialMatrix>
        create_matrix(const Context& context, SymbolTable& symbols, LocalizingMatrixIndex lmi,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        /**
         * If supplied input is symbol matrix associated with a monomial localizng matrix, extract that matrix.
         * Otherwise, returns nullptr.
         */
        static const LocalizingMatrix* as_monomial_localizing_matrix_ptr(const SymbolicMatrix& input) noexcept;

        friend class LocalizingMatrixCreationContext;
    };
}