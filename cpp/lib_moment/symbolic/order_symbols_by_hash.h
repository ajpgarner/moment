/**
 * order_symbols_by_hash.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial.h"
#include "symbol_table.h"

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

    /**
     * Factory, construct polynomial using op-hash ordering.
     */
     using ByHashPolynomialFactory = PolynomialFactoryImpl<CompareByOpHash>;

//    class ByHashPolynomialFactory : public PolynomialFactory {
//    private:
//        CompareByOpHash comparator;
//
//    public:
//        explicit ByHashPolynomialFactory(const SymbolTable& symbols, double zero_tolerance = 1.0)
//            : PolynomialFactory{symbols, zero_tolerance}, comparator{symbols} { }
//
//        [[nodiscard]] Polynomial operator()(Polynomial::storage_t&& data) const override {
//            return Polynomial{std::move(data), this->symbols, this->comparator, this->zero_tolerance};
//        }
//
//        void append(Polynomial &lhs, const Polynomial &rhs) const override {
//            lhs.append(rhs, this->comparator, this->zero_tolerance);
//        }
//
//        [[nodiscard]] bool less(const Monomial &lhs, const Monomial &rhs) const override {
//            return this->comparator(lhs, rhs);
//        }
//
//    };


}