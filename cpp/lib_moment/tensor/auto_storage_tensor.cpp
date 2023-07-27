/**
 * auto_storage_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "auto_storage_tensor.h"

#include <cassert>

#include <numeric>
#include <sstream>

namespace Moment::errors {
    bad_tensor bad_tensor_no_data_stored(const std::string &name) {
        std::stringstream errSS;
        errSS << name << " has no explicitly stored elements.";
        return bad_tensor{errSS.str()};
    }
}