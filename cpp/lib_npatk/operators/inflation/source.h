/**
 * source.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <set>

namespace NPATK {

    class Source  {
    public:
        const oper_name_t id;
        const std::set<oper_name_t> observables;

        Source(oper_name_t the_id, std::set<oper_name_t> connected_observables)
            : id{the_id}, observables{std::move(connected_observables)} { }
    };
}