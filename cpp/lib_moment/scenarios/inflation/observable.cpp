/**
 * observable.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "observable.h"
#include "utilities/ipow.h"

#include <cmath>

#include <algorithm>
#include <iterator>

namespace Moment::Inflation {
    size_t Observable::count_copies(size_t inflation_level) const {
        return this->singleton ? 1 : ipow(inflation_level, this->source_count);
    }

    size_t Observable::count_operators(size_t inflation_level) const {
        return this->operators() * this->count_copies(inflation_level);
    }

    oper_name_t Observable::flatten_index(const size_t inflation_level,
                                          const std::span<const oper_name_t> indices) const {
        oper_name_t index = 0;
        oper_name_t base = 1;
        for (oper_name_t n = 0; n < indices.size(); ++n) {
            index += indices[n] * base;
            base *= inflation_level;
        }
        return index;
    }

    bool Observable::contains_source(const oper_name_t source) const noexcept {
        auto find = std::lower_bound(this->sources.cbegin(), this->sources.cend(), source);
        if (find == this->sources.cend()) {
            return false;
        }
        return *find == source;
    }

    SourceIndex Observable::unflatten_index(const size_t inflation_level, oper_name_t index) const {
        SourceIndex output(this->source_count, 0);

        // Just return 0s if no inflation, or sources
        if (output.empty() || (inflation_level<=0)) {
            return output;
        }

        const auto signed_inflation_level = static_cast<oper_name_t>(inflation_level);

        for (size_t i = 0; i < this->source_count; ++i) {
            const auto next_index = static_cast<oper_name_t>(index / signed_inflation_level);
            const auto remainder = static_cast<oper_name_t>(index % signed_inflation_level);
            output[i] = remainder;
            index = next_index;
        }

        return output;
    }
}
