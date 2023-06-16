/**
 * monomial_comparator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "monomial.h"

namespace Moment {

    struct AbstractMonomialIdComparator {
        [[nodiscard]] virtual bool operator()(symbol_name_t lhs, symbol_name_t rhs) const noexcept = 0;
    };

    struct MonomialIdComparator_Less : public AbstractMonomialIdComparator {
        [[nodiscard]] bool operator()(symbol_name_t lhs, symbol_name_t rhs) const noexcept override {
            return lhs < rhs;
        }
    };

    /**
      * Comparator defining #1 < #1* < #2 < #2* < ... for symbol IDs.
      */
    struct IdLessComparator : public MonomialIdComparator_Less {
    public:
        constexpr bool operator()(const Monomial &lhs, const Monomial &rhs) const noexcept {
            if (lhs.id < rhs.id) {
                return true;
            } else if (lhs.id > rhs.id) {
                return false;
            }
            if (lhs.conjugated == rhs.conjugated) {
                return false;
            }
            return !lhs.conjugated; // true implies lhs a, rhs a*
        }

        [[nodiscard]] constexpr std::pair<uint64_t, uint64_t> key(const Monomial& lhs) const noexcept {
            return {static_cast<uint64_t>(lhs.id), static_cast<uint64_t>(lhs.conjugated ? 1 : 0)};
        }
    };


    struct MonomialIdComparator_More : public AbstractMonomialIdComparator {
        [[nodiscard]] bool operator()(symbol_name_t lhs, symbol_name_t rhs) const noexcept override {
            return lhs > rhs;
        }
    };

    /**
      * Comparator defining #3 < #3* < #2 < #2* < ... for symbol IDs.
      * Not quite the reverse ordering of IdLessComparator, because A < A* still.
      */
    struct IdMoreComparator : public MonomialIdComparator_More {
    public:
        [[nodiscard]] constexpr bool operator()(const Monomial &lhs, const Monomial &rhs) const noexcept {
            if (lhs.id > rhs.id) {
                return true;
            } else if (lhs.id < rhs.id) {
                return false;
            }
            if (lhs.conjugated == rhs.conjugated) {
                return false;
            }
            return !lhs.conjugated; // true implies lhs a, rhs a*
        }

        [[nodiscard]] constexpr std::pair<uint64_t, uint64_t> key(const Monomial& lhs) const noexcept {
            return {std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(lhs.id),
                    static_cast<uint64_t>(lhs.conjugated ? 1 : 0)};
        }
    };
}