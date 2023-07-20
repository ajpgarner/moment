/**
 * dynamic_bitset_fwd.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <cstdint>

#include <concepts>
#include <vector>

namespace Moment {
        template<std::unsigned_integral page_t = uint64_t,
                std::integral index_t = page_t,
                typename storage_t = std::vector<page_t>>
        class DynamicBitset;
}