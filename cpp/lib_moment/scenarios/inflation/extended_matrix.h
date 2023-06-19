/**
 * extended_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/monomial_matrix.h"

#include "factor_table.h"

#include <string>
#include <span>

namespace Moment::Inflation {


    /**
     * Scalar extensions of monomial moment matrix
     */
    class ExtendedMatrix : public MonomialMatrix {
    public:
        const size_t OriginalDimension;

    public:
        ExtendedMatrix(SymbolTable& symbols, Inflation::FactorTable& factors, double zero_tolerance,
                       const MonomialMatrix& source,
                       std::span<const symbol_name_t> extensions);

    };

}