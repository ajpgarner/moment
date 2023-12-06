/**
 * shortlex_hasher.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include <cmath>

#include <span>
#include <vector>

namespace Moment {

    /** Hashes will be stored as 64-bit integers. */
    using hash_t = uint64_t;

    /**
     * Dense hashing function, orders a sequence first by size, then lexicographically.
     */
    struct ShortlexHasher {
    public:
        /** The number of distinct unit operators. */
        const hash_t radix;

        /** A constant offset to add to the calculated hash. */
        const hash_t offset;

        /** Construct a shortlex hash function for supplied radix and offset. */
        constexpr explicit ShortlexHasher(hash_t r, hash_t o = 1) : radix{r}, offset{o} { }

        /** Calculate the hash of an operator sequence */
        [[nodiscard]] constexpr hash_t hash(const std::span<const oper_name_t> operator_string) const noexcept {
            // Initial hash and stride:
            hash_t hash = this->offset;
            hash_t multiplier = 1;

            // Hash from elements in operator string
            const hash_t len = operator_string.size();
            for (size_t n = 0; n < len; ++n) {
                hash += ((static_cast<hash_t>(operator_string[len-n-1]) + 1) * multiplier);
                multiplier *= this->radix;
            }
            return hash;
        }

        /** Short cut calculation of the hash for an isolated operator (sequence length 1) */
        [[nodiscard]] constexpr inline hash_t hash(const oper_name_t op) const noexcept {
            return this->offset + static_cast<hash_t>(op) + 1;
        }

        /** Calculate the hash of an operator sequence. */
        [[nodiscard]] inline hash_t operator()(const std::span<const oper_name_t> sequence)  const noexcept {
            return hash(sequence);
        }

        /** Calculate the hash of an operator sequence. */
        [[nodiscard]] inline hash_t operator()(std::initializer_list<oper_name_t> sequence)  const noexcept {
            return hash(std::span(sequence.begin(), sequence.size()));
        }

        /** The largest supported string. */
        [[nodiscard]] size_t longest_hashable_string() const {
            if (this->radix == 1) {
                // Hash is basically just string length for radix 1.
                return std::numeric_limits<size_t>::max() - this->offset;
            }
            return static_cast<size_t>(
                    static_cast<double>(std::numeric_limits<size_t>::digits) / std::log2(this->radix)
            );
        }
    };
}