/**
 * extended_matrix.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "extended_matrix.h"

namespace Moment {

    namespace {
        std::unique_ptr<SquareMatrix<SymbolExpression>>
        make_extended_matrix(SymbolTable& symbols, const OperatorMatrix &source,
                             std::span<const OperatorSequence> extensions) {
            throw std::logic_error{"Not yet implemented."};
        }
    }



    ExtendedMatrix::ExtendedMatrix(SymbolTable& symbols, const OperatorMatrix &source,
                                   std::span<const OperatorSequence> extensions)
        : SymbolicMatrix{source.context, symbols, make_extended_matrix(symbols, source, extensions)},
        OriginalDimension{source.Dimension()} {

    }

}