/**
 * NPA_generator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_collection.h"

#include <utility>

namespace NPATK {

    PartyInfo::PartyInfo(party_name_t id, std::string named, oper_name_t num_opers, Operator::Flags default_flags)
        : Party{id}, name{std::move(named)}  {

        this->operators.reserve(num_opers);
        for (oper_name_t o = 0; o < num_opers; ++o) {
            this->operators.emplace_back(o, *this, default_flags);
        }
    }

    OperatorCollection::OperatorCollection(std::vector<PartyInfo> &&in_party) noexcept
        : Parties{*this}, parties{std::move(in_party)} {

    }

    std::vector<PartyInfo> OperatorCollection::make_party_list(party_name_t num_parties, oper_name_t opers_per_party,
                                                               Operator::Flags default_flags) {
        std::vector<PartyInfo> output;
        output.reserve(num_parties);
        for (party_name_t p = 0; p < num_parties; ++p) {
            output.emplace_back(p, opers_per_party, default_flags);
        }
        return output;
    }

    std::vector<PartyInfo> OperatorCollection::make_party_list(std::initializer_list<oper_name_t> oper_per_party_list,
                                                               Operator::Flags default_flags) {
        std::vector<PartyInfo> output;
        output.reserve(oper_per_party_list.size());

        party_name_t p = 0;
        for (auto count : oper_per_party_list) {
            output.emplace_back(p, count, default_flags);
            ++p;
        }
        return output;
    }

}