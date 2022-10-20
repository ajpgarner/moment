/**
 * shortlex_hasher.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "hashed_sequence.h"
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

    static_assert(HashFunction<ShortlexHasher>);

}