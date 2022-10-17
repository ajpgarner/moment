/**
 * hashable_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <vector>

namespace NPATK {

    struct ShortlexHasher {
    public:
        /** The number of distinct unit operators */
        const size_t radix;

        /** Construct a shortlex hash function for supplied radix */
        explicit ShortlexHasher(size_t r) : radix{r} { }

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] size_t hash(const std::vector<oper_name_t>& sequence) const noexcept;

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] inline size_t operator()(const std::vector<oper_name_t>& sequence)  const noexcept {
            return hash(sequence);
        }
    };

    class HashedSequence {
    public:
        std::vector<oper_name_t> operators{};

        size_t hash = 0;

        /** Construct empty sequence */
        constexpr HashedSequence() = default;

        /** Construct a sequence, from a list of operators and its hash  */
        constexpr HashedSequence(std::vector<oper_name_t>&& oper_ids, size_t hash)
                : operators{std::move(oper_ids)}, hash{hash} { }

        /** Construct a sequence, from a list of operators and its hash  */
        HashedSequence(std::vector<oper_name_t>&& oper_ids, const ShortlexHasher& hasher)
            : operators{std::move(oper_ids)} { this->hash = hasher(this->operators); }

        /** True, if the operator string is empty */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        /** The length of the operator string */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        /** Ordering by hash value (i.e. shortlex) */
        [[nodiscard]] constexpr bool operator<(const HashedSequence& rhs) const noexcept {
            return this->hash < rhs.hash;
        }
    };

}