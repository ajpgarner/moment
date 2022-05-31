/**
 * read_symbol_or_fail.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "read_symbol_or_fail.h"
#include "../utilities/reporting.h"

namespace NPATK::mex {

    SymbolExpression read_symbol_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::StringArray &matrix,
                                         size_t index_i, size_t index_j) {

        if (!matrix[index_i][index_j].has_value()) {
            std::stringstream errMsg;
            errMsg << "Element [" << index_i << ", " << index_j << " was empty.";
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }
        try {
            NPATK::SymbolExpression elem{matlab::engine::convertUTF16StringToUTF8String(matrix[index_i][index_j])};
            return elem;
        } catch (const SymbolExpression::SymbolParseException& e) {
            std::stringstream errMsg;
            errMsg << "Error converting element [" << index_i << ", " << index_j << ": " << e.what();
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }

    }
}