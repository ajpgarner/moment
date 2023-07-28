/**
 * read_symbol_or_fail.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "read_symbol_or_fail.h"

#include "../utilities/reporting.h"

#include "utilities/utf_conversion.h"

namespace Moment::mex {

    Monomial read_symbol_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::MATLABString& string) {
        if (!string.has_value()) {
            std::stringstream errMsg;
            errMsg << "Cannot read empty string as a symbol.";
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }
        try {
            Monomial elem{UTF16toUTF8Convertor{}(string)};
            return elem;
        } catch (const Monomial::SymbolParseException& e) {
            std::stringstream errMsg;
            errMsg << "Error in conversion: " << e.what();
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }
    }

    Monomial read_symbol_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::StringArray &matrix,
                                 size_t index_i, size_t index_j) {

        if (!matrix[index_i][index_j].has_value()) {
            std::stringstream errMsg;
            errMsg << "Element [" << index_i << ", " << index_j << " was empty.";
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }
        try {
            Monomial elem{UTF16toUTF8Convertor{}(matrix[index_i][index_j])};
            return elem;
        } catch (const Monomial::SymbolParseException& e) {
            std::stringstream errMsg;
            errMsg << "Error converting element [" << index_i << ", " << index_j << ": " << e.what();
            throw_error(engine, errors::bad_symbol, errMsg.str());
        }

    }
}