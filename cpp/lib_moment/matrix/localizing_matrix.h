/**
 * localizing_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "localizing_matrix_index.h"
#include "operator_matrix.h"

namespace Moment {


    class LocalizingMatrix : public OperatorMatrix {
    public:
        /**
         * "Index" of this localizing matrix, containing its depth and localizing word.
         */
        const LocalizingMatrixIndex Index;

    public:
        /**
          * Constructs a localizing matrix at the requested hierarchy depth (level) for the supplied context,
          * with the supplied word.
          * @param context The setting/scenario.
          * @param symbols Source of existing symbols, sink for any new symbols first appearing in this matrix.
          * @param lmi Index, describing the hierarchy depth and localizing word.
          */
        LocalizingMatrix(const Context& context, SymbolTable& symbols, LocalizingMatrixIndex lmi);

        /**
         * Constructs a localizing matrix at the requested hierarchy depth (level) for the supplied context,
         * with the supplied word.
         * @param context The setting/scenario.
         * @param symbols Source of existing symbols, sink for any new symbols first appearing in this matrix.
         * @param level The hierarchy depth.
         * @param word The localizing word.
         */
        LocalizingMatrix(const Context& context, SymbolTable& symbols, size_t level, OperatorSequence&& word)
            : LocalizingMatrix(context, symbols, LocalizingMatrixIndex{context, level, std::move(word)}) { }

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
    };
}