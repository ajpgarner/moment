/**
 * operator_collection.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"

#include <utility>

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

    Context::Context(std::vector<PartyInfo> &&in_party) noexcept
        : Parties{*this}, parties{std::move(in_party)}, total_operator_count(0)  {
        for ( auto& party : parties) {
            assert(party.offset() == this->total_operator_count);
            this->total_operator_count += party.size();
        }
    }

    std::vector<PartyInfo> Context::make_party_list(party_name_t num_parties, oper_name_t opers_per_party,
                                                    Operator::Flags default_flags) {
        std::vector<PartyInfo> output;
        output.reserve(num_parties);
        size_t global = 0;
        for (party_name_t p = 0; p < num_parties; ++p) {
            output.emplace_back(p, opers_per_party, global, default_flags);
            global += opers_per_party;
        }
        return output;
    }

    std::vector<PartyInfo> Context::make_party_list(std::initializer_list<oper_name_t> oper_per_party_list,
                                                    Operator::Flags default_flags) {
        std::vector<PartyInfo> output;
        output.reserve(oper_per_party_list.size());

        party_name_t p = 0;
        size_t global = 0;
        for (auto count : oper_per_party_list) {
            output.emplace_back(p, count, global, default_flags);
            global += count;
            ++p;
        }
        return output;
    }

    std::pair<std::vector<Operator>::iterator, bool>
    Context::additional_simplification(std::vector<Operator>::iterator start,
                                       std::vector<Operator>::iterator end) const noexcept {
        // Do nothing on empty set
        if (start == end) {
            return {end, false};
        }

        // Look for mutually exclusive operators
        auto lhs_iter = start;
        auto rhs_iter = start + 1;

        while (rhs_iter != end) {
            // Only do comparison if operators are in same party...
            if (lhs_iter->party.id == rhs_iter->party.id) {
                assert(lhs_iter->party.id < this->parties.size());
                const auto& party = this->parties[lhs_iter->party.id];
                // If mutually-exclusive operators are found next to each other, whole sequence is zero.
                if (party.exclusive(lhs_iter->id, rhs_iter->id)) {
                    return {start, true};
                }
            }

            // Advance iterators
            lhs_iter = rhs_iter;
            ++rhs_iter;
        }

        return {end, false};
    }

}