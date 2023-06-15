/**
 * order_symbols_by_hash.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial_factory.h"

namespace Moment {

    /**
     * Order first by operator hash of forward sequence, then by conjugation sequence.
     * This is not quite the same as op-hash order; because it guarantees complex conjugate strings are adjacent.
     */
    struct CompareByOpHash {
    public:
        const SymbolTable& symbolTable;

        explicit CompareByOpHash(const SymbolTable& symbolTable)
                : symbolTable{symbolTable} { }

        [[nodiscard]] bool operator()(const Monomial& lhs, const Monomial& rhs) const noexcept;
    };


    struct ByHashPolynomialFactory_Name {
        constexpr static char name[] = "Sort by hash";
    };

    /**
     * Factory, construct polynomial using op-hash ordering.
     */
     using ByHashPolynomialFactory = PolynomialFactoryImpl<CompareByOpHash, ByHashPolynomialFactory_Name>;
}