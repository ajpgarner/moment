/**
 * compare_by_op_hash.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "symbol_expression.h"

namespace Moment {
    class SymbolTable;

    /**
     * Order first by operator hash of forward sequence, then by conjugation sequence.
     * This is not quite the same as op-hash order; because it guarantees complex conjugate strings are adjacent.
     */
    struct CompareByOpHash {
    public:
        const SymbolTable& symbolTable;

        explicit CompareByOpHash(const SymbolTable& symbolTable)
            : symbolTable{symbolTable} { }

        [[nodiscard]] bool operator()(const SymbolExpression& lhs, const SymbolExpression& rhs) const noexcept;

    };
}