/**
 * observable.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <set>
#include <vector>

namespace NPATK {
    class Observable  {
    public:
        const oper_name_t id;
        const size_t outcomes;
        const std::set<oper_name_t> sources;
        const size_t source_count;
        const bool singleton;

    public:
        Observable(oper_name_t the_id, size_t outcome_count, std::set<oper_name_t> connected_sources, bool single)
            : id{the_id}, outcomes{outcome_count},
              sources{std::move(connected_sources)}, source_count{sources.size()}, singleton{single} { }

        /** Number of versions of the observable at a given inflation leve.l */
        [[nodiscard]] size_t count_copies(size_t inflation_level) const;

        /** Number of operators associated with this observable, at a given inflation level. */
        [[nodiscard]] size_t count_operators(size_t inflation_level) const;

        /** Convert global index to vector of source indices. */
        [[nodiscard]] std::vector<oper_name_t> unflatten_index(size_t inflation_level, oper_name_t index) const;

        /** Is this a projective measurement (cf. a generic moment)? */
        [[nodiscard]] bool projective() const noexcept { return outcomes != 0; }

        /** The number of operators to associate with this observable */
        [[nodiscard]] size_t operators() const noexcept {
            return (outcomes != 0) ? (outcomes - 1) : 1;
        }
    };
}