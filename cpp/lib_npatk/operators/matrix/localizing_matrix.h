/**
 * localizing_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../operator_sequence.h"
#include "operator_matrix.h"
#include "moment_matrix.h"

namespace NPATK {

    class LocalizingMatrix : public OperatorMatrix {
    public:
        const OperatorSequence word;

    public:
        LocalizingMatrix(const Context& context, SymbolTable& symbols, const OperatorSequence&& seedWord);

    };
}