/**
 * operator_collection.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"
#include "operator_sequence.h"

#include <utility>

namespace NPATK {
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

    size_t Context::hash(const OperatorSequence &sequence) const noexcept {
        size_t hash = 1;
        size_t multiplier = 1;

        if (sequence.zero()) {
            return 0;
        }

        for (size_t n = 0; n < sequence.size(); ++n) {
            const auto& oper = sequence[sequence.size()-n-1];
            size_t global_index = 1 + this->parties[oper.party.id].offset() + oper.id;
            hash += (global_index * multiplier);
            multiplier *= (1+this->total_operator_count);
        }
        return hash;
    }
}