/**
 * extended_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "matrix/operator_matrix.h"

#include <span>

namespace Moment {


    class ExtendedMatrix : public SymbolicMatrix {
    public:
        const size_t OriginalDimension;

    public:
        ExtendedMatrix(SymbolTable& symbols, const OperatorMatrix& source,
                       std::span<const OperatorSequence> extensions);

    };

}