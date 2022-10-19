/**
 * raw_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "/operators/hashed_sequence.h"

#include <iosfwd>
#include <vector>

namespace NPATK {

    struct RawSequence : public HashedSequence {
    public:
        symbol_name_t raw_id = 0;
        size_t conjugate_hash = 0;
        symbol_name_t conjugate_id = 0;

    public:
        /** Construct empty raw sequence */
        constexpr RawSequence() = default;

        /** Construct a raw sequence, from a list of operators, its hash and its assigned symbol id. */
        constexpr RawSequence(std::vector<oper_name_t>&& oper_ids, size_t hash, symbol_name_t name)
                : HashedSequence{std::move(oper_ids), hash}, raw_id{name} { }

        /** True if the operator is its own conjugate */
        [[nodiscard]] constexpr bool self_adjoint() const noexcept { return this->raw_id == this->conjugate_id; }

        /** Output this operator sequence */
        friend std::ostream& operator<<(std::ostream& os, const RawSequence& rs);
    };
}