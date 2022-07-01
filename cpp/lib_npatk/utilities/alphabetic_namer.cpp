/**
 * alphabetic_namer.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "alphabetic_namer.h"

#include <cassert>
#include <cmath>

namespace NPATK {

    size_t AlphabeticNamer::strlen(size_t id) noexcept {
        constexpr double one_over_ln26 = 0.30692767643013485;
        return 1 + static_cast<size_t>(std::floor(one_over_ln26 * std::log((static_cast<double>(id) * 25. / 26.) + 1.)));
    }

    size_t AlphabeticNamer::level_offset(size_t level) noexcept {
        return static_cast<size_t>(std::round((26. * (std::pow(26, static_cast<double>(level)) - 1.)) / 25.));
    }

    std::string AlphabeticNamer::operator()(size_t id) const {
        // Quick exit for short strings
        if (id < 26) [[likely]] {
            return std::string{static_cast<char>((this->upper_case ? 'A' : 'a') + static_cast<char>(id))};
        }

        // Otherwise, use excel ordering (A-Z, AA-ZZ, AAA-ZZZ, etc.)

        const size_t len = strlen(id);
        assert(len > 0);
        std::string output(len, '.');
        size_t beyond = id - level_offset(len-1);

        for (size_t index = 0; index < len; ++index) {
            auto div = std::div(static_cast<long>(beyond), 26L);
            beyond = div.quot;
            output[len-index-1] = static_cast<char>((this->upper_case ? 'A' : 'a') + static_cast<char>(div.rem));
        }

        return output;
    }

}