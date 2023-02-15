/**
 * persistent_storage.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "persistent_storage.h"

#include <sstream>

namespace Moment {
    namespace errors {
        std::string bad_signature_error::make_msg(uint32_t actual_sig, uint32_t expected_sig) {
            std::stringstream ss;
            ss << "Bad signature \"" << actual_sig << "\" - expected signature \"" << expected_sig << "\"";
            return ss.str();
        }

        std::string not_found_error::make_msg(uint32_t supplied_id) {
            std::stringstream ss;
            ss << "Object with ID \"" << supplied_id << "\" not found.";
            return ss.str();
        }
    }
}