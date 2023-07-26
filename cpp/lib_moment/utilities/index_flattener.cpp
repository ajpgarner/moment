/**
 * index_flattener.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "index_flattener.h"

#include <numeric>
#include <span>
#include <stdexcept>

namespace Moment {
    namespace {
        std::vector<size_t> calculate_index_sizes(const std::vector<std::vector<size_t>> &indices) {
            std::vector<size_t> output;
            output.reserve(indices.size());
            for (const auto &index: indices) {
                output.emplace_back(index.size());
            }
            return output;
        }
    }

    IndexFlattener::Index IndexFlattener::FlattenedIndexIter::index() const {
        Index output;

        const auto& raw_index = *index_iter;
        output.reserve(raw_index.size());
        for (size_t dim_index = 0; dim_index < raw_index.size(); ++dim_index) {
            output.emplace_back(range->indices_per_dimensions[dim_index][raw_index[dim_index]]);
        }

        return output;
    }

    size_t IndexFlattener::FlattenedIndexIter::operator*() const {
        const auto indices = this->index();
        return range->object.index_to_offset_no_checks(indices);
    }

    IndexFlattener::IndexFlattener(ObjectType object, std::vector<std::vector<size_t>> indices)
            : object{std::move(object)},
              indices_per_dimensions{std::move(indices)},
              index_sizes{calculate_index_sizes(indices_per_dimensions)} {
    }

    size_t IndexFlattener::size() const noexcept {
        if (this->index_sizes.empty()) [[unlikely]] {
            return 0;
        }
        return std::reduce(this->index_sizes.begin(), this->index_sizes.end(),
                           static_cast<size_t>(1), std::multiplies{});
    }

    bool IndexFlattener::empty() const noexcept {
        // If no indices, we are empty.
        if (this->index_sizes.empty()) {
            return true;
        }

        // If any index size is zero, we are empty.
        return std::any_of(this->index_sizes.begin(), this->index_sizes.end(), [](size_t val) { return val == 0; });
    }

}
