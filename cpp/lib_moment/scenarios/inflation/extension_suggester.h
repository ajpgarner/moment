/**
 * extension_suggester.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "utilities/dynamic_bitset.h"

#include <set>

namespace Moment {
    class SymbolTable;
    class SymbolicMatrix;
    class MomentMatrix;
}

namespace Moment::Inflation {

    class FactorTable;

    class ExtensionSuggester {
    private:
        const SymbolTable& symbols;
        const FactorTable& factors;

        const size_t max_extensions = 100;

    public:
        explicit ExtensionSuggester(const SymbolTable& symbols, const FactorTable& factors);

        [[nodiscard]] std::set<symbol_name_t> operator()(const MomentMatrix& matrix) const;

        /**
         * Bitset, with set bits corresponding to non-fundamental symbols present in matrix.
         */
        [[nodiscard]] DynamicBitset<uint64_t> nonfundamental_symbols(const SymbolicMatrix &matrix) const;
    };

}