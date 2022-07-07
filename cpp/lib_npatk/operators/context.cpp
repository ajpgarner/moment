/**
 * operator_collection.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"
#include "operator_sequence.h"

#include <iostream>
#include <sstream>
#include <utility>

namespace NPATK {
    Context::Context(std::vector<Party> &&in_party) noexcept
        : Parties{*this}, parties{std::move(in_party)},
          total_measurement_count{0}, total_operator_count{0} {

        party_name_t party_count = 0;
        for ( auto& party : parties) {
            party.global_operator_offset = this->total_operator_count;
            party.set_offsets(party_count, static_cast<mmt_name_t>(this->total_measurement_count));
            this->total_operator_count += party.size();
            this->total_measurement_count += party.measurements.size();
            this->global_to_party.insert(this->global_to_party.end(), party.measurements.size(), party_count);
            ++party_count;
        }
    }

    void Context::add_party(Party info) {
        const auto party_count = static_cast<party_name_t>(this->parties.size());

        this->parties.emplace_back(std::move(info));
        auto& party = this->parties.back();
        party.context = this;
        party.global_operator_offset = this->total_operator_count;
        party.set_offsets(party_count, static_cast<mmt_name_t>(this->total_measurement_count));
        this->total_operator_count += party.size();
        this->total_measurement_count += party.measurements.size();
        this->global_to_party.insert(this->global_to_party.end(), party.measurements.size(), party_count);
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
            if (lhs_iter->party == rhs_iter->party) {
                assert(lhs_iter->party < this->parties.size());
                const auto& party = this->parties[lhs_iter->party];
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
            size_t global_index = 1 + this->parties[oper.party].global_operator_offset + oper.id;
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

    std::string Context::format_sequence(const OperatorSequence &seq) const {
        if (seq.zero()) {
            return "0";
        }
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;
        const size_t party_size = this->parties.size();
        bool done_once = false;
        for (const auto& oper : seq) {
            if (done_once) {
                ss << ";";
            } else {
                done_once = true;
            }

            if (oper.party >= party_size) {
                return "BadSequence";
            }
            const auto& party = this->parties[oper.party];
            if (party_size >= 1) {
                ss << party.name << ".";
            }
            party.format_operator(ss, oper);
        }
        return ss.str();
    }

    std::string Context::format_sequence(const std::span<const PMOIndex> indices, const bool zero) const {
        if (zero) {
            return "0";
        }

        if (indices.empty()) {
            return "1";
        }

        std::stringstream ss;
        bool done_once = false;
        for (const auto& index : indices) {
            assert(index.party < this->parties.size());
            const auto& party = this->parties[index.party];
            assert(index.mmt < party.measurements.size());
            const auto& mmt = party.measurements[index.mmt];
            if (done_once) {
                ss << ";";
            }
            ss << party.name << "." << mmt.name << index.outcome;
            done_once = true;
        }

        return ss.str();
    }

    void Context::reenumerate() {
        this->total_operator_count = 0;
        this->total_measurement_count = 0;
        for (size_t index = 0; index < this->parties.size(); ++index) {
            this->parties[index].global_operator_offset = this->total_operator_count;
            this->parties[index].set_offsets(static_cast<party_name_t>(index),
                                             static_cast<mmt_name_t>(this->total_measurement_count));
            this->total_operator_count += this->parties[index].operators.size();
            this->total_measurement_count += this->parties[index].measurements.size();
        }
    }

    std::vector<size_t> Context::measurements_per_party() const noexcept {
        std::vector<size_t> output;
        for (const auto& p : this->parties) {
            output.push_back(p.measurements.size());
        }
        return output;
    }

    PMIndex Context::global_index_to_PM(size_t global_index) const noexcept {
        assert (global_index < this->global_to_party.size());
        auto party_id = this->global_to_party[global_index];
        auto mmt_id = static_cast<mmt_name_t>(global_index - this->parties[party_id].global_mmt_offset);

        assert(mmt_id >= 0);
        return PMIndex{party_id, mmt_id, static_cast<mmt_name_t>(global_index)};
    }

}