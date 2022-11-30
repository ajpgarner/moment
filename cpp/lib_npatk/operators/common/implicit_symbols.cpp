/**
 * implicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
/**
 * implicit_symbols.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "implicit_symbols.h"
#include "operators/matrix_system.h"
#include "operators/matrix/symbol_table.h"

#include "utilities/combinations.h"

#include <algorithm>

namespace NPATK {
    std::span<const PMODefinition> ImplicitSymbols::get(const std::span<const size_t> mmtIndex) const {
        if (mmtIndex.size() > this->MaxSequenceLength) {
            throw errors::bad_implicit_symbol("Cannot look up sequences longer than the max sequence length.");
        }

        auto [first, last] = this->indices.access(mmtIndex);
        if ((first < 0) || (first >= last)) {
            return {tableData.begin(), 0};
        }
        assert(last <= tableData.size());
        return {tableData.begin() + first, static_cast<size_t>(last - first)};
    }
}