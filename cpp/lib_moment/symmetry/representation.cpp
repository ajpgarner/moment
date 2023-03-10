/**
 * representation.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "representation.h"

namespace Moment {
    namespace  {
        size_t extract_dim(const std::vector<repmat_t>& e) {
            if (e.empty()) {
                return 0;
            }
            assert(e.front().cols() == e.front().rows());
            return static_cast<size_t>(e.front().rows());
        }

        bool debug_check_all_same_dim(const std::vector<repmat_t>& elems, const size_t dim) {
            for (auto& elem : elems) {
                if (elem.rows() != dim) {
                    return false;
                }
                if (elem.cols() != dim) {
                    return false;
                }
            }
            return true;
        }
    }

    Representation::Representation(std::vector<repmat_t> &&entries)
        : dimension(extract_dim(entries)), elements(std::move(entries)) {

        assert(debug_check_all_same_dim(elements, dimension));
    }
}