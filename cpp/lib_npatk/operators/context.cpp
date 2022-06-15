/**
 * operator_collection.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"
#include "operator_sequence.h"

#include <iostream>
#include <utility>

namespace NPATK {
    Context::Context(std::vector<PartyInfo> &&in_party) noexcept
        : Parties{*this}, parties{std::move(in_party)}, total_operator_count{0}  {
        for ( auto& party : parties) {
            party.global_offset = this->total_operator_count;
            this->total_operator_count += party.size();
        }
    }


    void Context::add_party(PartyInfo info) {
        this->parties.emplace_back(std::move(info));
        auto& party = this->parties.back();
        party.global_offset = this->total_operator_count;
        this->total_operator_count += party.size();
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
            size_t global_index = 1 + this->parties[oper.party.id].global_offset + oper.id;
            hash += (global_index * multiplier);
            multiplier *= (1+this->total_operator_count);
        }
        return hash;
    }

    std::ostream &operator<<(std::ostream &os, const Context &context) {
        os << context.parties.size() << ((context.parties.size() == 1) ? " party" : " parties") << ".\n";
        for (const auto& party : context.parties) {
            os << party << "\n";
        }
        os << context.total_operator_count << ((context.total_operator_count == 1) ? " operator" : " operators")
           << " in total.\n";

        return os;
    }

}