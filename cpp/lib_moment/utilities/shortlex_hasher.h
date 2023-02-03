/**
 * shortlex_hasher.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <span>
#include <vector>

namespace Moment {

    /**
     * Dense hasing function, orders a sequence first by size, then lexicographically.
     */
    struct ShortlexHasher {
    public:
        /** The number of distinct unit operators. */
        const size_t radix;

        /** A constant offset to add to the calculated hash. */
        const size_t offset;

        /** Construct a shortlex hash function for supplied radix and offset. */
        explicit ShortlexHasher(size_t r, size_t o = 1) : radix{r}, offset{o} { }

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] size_t hash(std::span<const oper_name_t> sequence) const noexcept;

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] inline size_t operator()(std::span<const oper_name_t> sequence)  const noexcept {
            return hash(sequence);
        }

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] inline size_t operator()(std::initializer_list<oper_name_t> sequence)  const noexcept {
            return hash(std::vector(sequence));
        }

        /** The largest supported string. */
        [[nodiscard]] size_t longest_hashable_string() const;
    };
}