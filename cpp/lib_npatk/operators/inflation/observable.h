/**
 * observable.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <set>

namespace NPATK {
    class Observable  {
    public:
        const oper_name_t id;
        const size_t outcomes;
        const std::set<oper_name_t> sources;

    public:
        Observable(oper_name_t the_id, size_t outcome_count, std::set<oper_name_t> connected_sources)
            : id{the_id}, outcomes{outcome_count}, sources{std::move(connected_sources)} { }

        /** Number of versions of the observable at a given inflation level */
        [[nodiscard]] size_t count_copies(size_t inflation_level) const;

        /** Number of operators associated with this observable, at a given inflation level */
        [[nodiscard]] size_t count_operators(size_t inflation_level) const;
    };
}