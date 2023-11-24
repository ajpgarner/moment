/**
 * order_symbols_by_hash.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial_factory.h"
#include "monomial_comparator.h"

namespace Moment {

    /**
     * Order first by operator hash of forward sequence, then by conjugation sequence.
     * This is not quite the same as op-hash order; because it guarantees complex conjugate strings are adjacent.
     */
    struct CompareByOpHash : public AbstractMonomialIdComparator {
    public:
        const SymbolTable& symbolTable;

        explicit CompareByOpHash(const SymbolTable& symbolTable)
                : symbolTable{symbolTable} { }

        [[nodiscard]] bool operator()(symbol_name_t lhs, symbol_name_t rhs) const noexcept override;

        [[nodiscard]] bool operator()(const Monomial& lhs, const Monomial& rhs) const noexcept;

        [[nodiscard]] std::pair<uint64_t, uint64_t> key(const Monomial& lhs) const noexcept;
    };


    struct ByHashPolynomialFactory_Name {
        constexpr static char name[] = "Sort by hash";
    };

    /**
     * Factory, construct polynomial using op-hash ordering.
     */
     class ByHashPolynomialFactory : public PolynomialFactoryImpl<CompareByOpHash, ByHashPolynomialFactory_Name> {
     public:

         explicit ByHashPolynomialFactory(const SymbolTable& symbols, double zero_tolerance = 1.0)
                 : PolynomialFactoryImpl{symbols, zero_tolerance, symbols} { }

         [[nodiscard]] size_t maximum_degree(const Polynomial& poly) const override;
     };
}