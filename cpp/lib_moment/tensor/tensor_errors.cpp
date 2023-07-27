/**
 * tensor_errors.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "tensor_errors.h"

#include <sstream>

namespace Moment::errors {

    bad_tensor_index bad_tensor_index::offset_out_of_range(const std::string &indexExpr, const std::string &maxExpr) {
        std::stringstream errSS;
        errSS << "Offset '" << indexExpr << "' is not valid (element count: " << maxExpr << ").";
        return bad_tensor_index{errSS.str()};
    }

    bad_tensor_index
    bad_tensor_index::index_out_of_range(const std::string &dimensionExpr, const std::string &indexExpr,
                                         const std::string &maxExpr) {
        std::stringstream errSS;
        errSS << "Index '" << indexExpr << "' for dimension " << dimensionExpr
              << " was not valid (dimension length: " << maxExpr << ").";

        return bad_tensor_index(errSS.str());
    }

    bad_tensor_index bad_tensor_index::bad_dimension_count(const size_t actual, const size_t expected) {
        std::stringstream errSS;
        errSS << "Expected index of " << actual << " dimensions, but " << actual << " provided.";
        return bad_tensor_index(errSS.str());
    }

    bad_tensor_index bad_tensor_index::wrong_order(size_t d, const std::string& min, const std::string& max) {
        std::stringstream errSS;
        errSS << "Invalid splice dimension " << d << ": "
              << "Index " << min << " must be smaller than index " << max << ".";
        return bad_tensor_index{errSS.str()};
    }

}