/**
 * observable.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "utilities/small_vector.h"
#include "utilities/set_to_vector.h"

#include <algorithm>
#include <set>
#include <span>
#include <vector>

namespace Moment::Inflation {

    using SourceIndex = SmallVector<oper_name_t, 4>;

    class Observable  {
    public:
        const oper_name_t id;
        const size_t outcomes;
        const std::vector<oper_name_t> sources;
        const size_t source_count;
        const bool singleton;

    public:
        Observable(oper_name_t the_id, size_t outcome_count,
                   const std::set<oper_name_t>& connected_sources, bool single)
            : id{the_id}, outcomes{outcome_count},
              sources{set_to_vector(connected_sources)}, source_count{sources.size()}, singleton{single} { }

        /** Number of versions of the observable at a given inflation leve.l */
        [[nodiscard]] size_t count_copies(size_t inflation_level) const;

        /** Number of operators associated with this observable, at a given inflation level. */
        [[nodiscard]] size_t count_operators(size_t inflation_level) const;

        /** True if an observable is linked to a source. */
        [[nodiscard]] bool contains_source(oper_name_t source) const noexcept;

        /** Convert vector of source indices to global index.
         * Note: this uses a first-index-contiguous (col-major) scheme. */
        [[nodiscard]] oper_name_t flatten_index(size_t inflation_level, std::span<const oper_name_t> index) const;

        /** Convert global index to vector of source indices.
         * Note: this uses a first-index-contiguous (col-major) scheme. */
        [[nodiscard]] SourceIndex unflatten_index(size_t inflation_level, oper_name_t index) const;

        /** Is this a projective measurement (cf. a generic moment)? */
        [[nodiscard]] bool projective() const noexcept { return outcomes != 0; }

        /** The number of operators to associate with this observable */
        [[nodiscard]] size_t operators() const noexcept {
            return (outcomes != 0) ? (outcomes - 1) : 1;
        }

    };
}