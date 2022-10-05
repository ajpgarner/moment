/**
 * raw_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <vector>

namespace NPATK {
    struct RawSequence {
        std::vector<oper_name_t> operators{};
        size_t hash = 0;
        symbol_name_t raw_id = 0;

        constexpr RawSequence() = default;

        constexpr RawSequence(std::vector<oper_name_t>&& oper_ids, size_t hash, symbol_name_t name)
                : operators{std::move(oper_ids)}, hash{hash}, raw_id{name} { }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }
    };
}