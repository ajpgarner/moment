/**
 * party_info.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "party_info.h"

namespace NPATK {
    PartyInfo::PartyInfo(party_name_t id, std::string named, oper_name_t num_opers,
                         size_t offset,
                         Operator::Flags default_flags)
            : Party{id}, name{std::move(named)}, global_offset{offset} {

        this->operators.reserve(num_opers);
        for (oper_name_t o = 0; o < num_opers; ++o) {
            this->operators.emplace_back(o, *this, default_flags);
        }
    }
}