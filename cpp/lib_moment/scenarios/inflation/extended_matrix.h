/**
 * extended_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "matrix/operator_matrix.h"
#include "matrix/moment_matrix.h"

#include "factor_table.h"

#include <span>

namespace Moment::Inflation {


    /**
     * Scalar extensions of moment matrix
     */
    class ExtendedMatrix : public SymbolicMatrix {
    public:
        const size_t OriginalDimension;

    private:
        std::string description_string;

    public:
        ExtendedMatrix(SymbolTable& symbols, Inflation::FactorTable& factors,
                       const MomentMatrix& source,
                       std::span<const symbol_name_t> extensions);

        std::string description() const override {
            return description_string;
        }
    };

}