/**
 * locality_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "locality_context.h"

#include "../operator_sequence.h"

#include "party.h"

#include <iostream>
#include <sstream>
#include <utility>

namespace NPATK {
    LocalityContext::LocalityContext(std::vector<Party> &&in_party) noexcept
        : Parties{*this}, parties{std::move(in_party)}, total_measurement_count{0} {
        this->total_measurement_count = 0;

        party_name_t party_index = 0;
        oper_name_t total_operator_count  = 0;

        this->mmts_per_party.reserve(this->parties.size());
        this->ops_per_party.reserve(this->parties.size());

        for (auto &party: parties) {
            party.context = this;
            party.global_operator_offset = total_operator_count;
            party.set_offsets(party_index,
                              total_operator_count,
                              static_cast<mmt_name_t>(total_measurement_count));

            this->total_measurement_count += party.measurements.size();
            this->global_to_party.insert(this->global_to_party.end(), party.measurements.size(), party_index);

            // Register operators...
            mmt_name_t mmt_index = 0;
            for (const auto& mmt : party.measurements) {
                for (size_t oper_index = 0; oper_index < mmt.num_operators(); ++oper_index) {
                    this->operators.emplace_back(total_operator_count, party.id(),
                                                 mmt.projective ? Operator::Flags::Idempotent : Operator::Flags::None);
                    this->global_to_local_indices.emplace_back(party_index, mmt_index, oper_index);
                    ++total_operator_count;
                }
                ++mmt_index;
            }

            this->mmts_per_party.emplace_back(party.measurements.size());
            this->ops_per_party.emplace_back(party.party_operator_count);

            ++party_index;
        }
    }



    bool LocalityContext::additional_simplification(std::vector<Operator>& op_sequence) const {
        // Do nothing on empty set
        if (op_sequence.empty()) {
            return false;
        }

        // Look for mutually exclusive operators
        auto lhs_iter = op_sequence.begin();
        auto rhs_iter = op_sequence.begin() + 1;

        while (rhs_iter != op_sequence.end()) {
            // Only do comparison if operators are in same party, and party is well defined...
            if ((lhs_iter->party >= 0) && (lhs_iter->party < this->parties.size())
                    && (lhs_iter->party == rhs_iter->party)) {
                const auto& party = this->parties[lhs_iter->party];
                // If mutually-exclusive operators are found next to each other, whole sequence is zero.
                if (party.mutually_exclusive(*lhs_iter, *rhs_iter)) {
                    op_sequence.clear();
                    return true;
                }
            }

            // Advance iterators
            lhs_iter = rhs_iter;
            ++rhs_iter;
        }

        return false;
    }

    PMIndex LocalityContext::global_index_to_PM(size_t global_index) const noexcept {
        assert (global_index < this->global_to_party.size());
        auto party_id = this->global_to_party[global_index];
        auto mmt_id = static_cast<mmt_name_t>(global_index - this->parties[party_id].global_measurement_offset);

        assert(mmt_id >= 0);
        return PMIndex{party_id, mmt_id, static_cast<mmt_name_t>(global_index)};
    }

    void LocalityContext::get_global_mmt_index(std::vector<PMIndex> &pm_index) const noexcept {
        for (auto& pm : pm_index) {
            assert (pm.party < this->parties.size());
            assert (pm.mmt < this->parties[pm.party].measurements.size());
            pm.global_mmt = static_cast<mmt_name_t>(this->parties[pm.party].global_measurement_offset + pm.mmt);
        }
    }


    std::string LocalityContext::format_sequence(const OperatorSequence &seq) const {
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
            ss << party.format_operator(oper);
        }
        return ss.str();
    }

    std::string LocalityContext::format_sequence(const std::span<const PMOIndex> indices, const bool zero) const {
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

    std::string LocalityContext::to_string() const {
        std::stringstream ss;

        const size_t party_count = this->parties.size();
        ss << "Locality setting with " << party_count << ((party_count== 1 ? " party" : " parties")) << ".\n";

        for (const auto& party : this->parties) {
            ss << party << "\n";
        }

        const size_t total_operator_count = this->operators.size();
        ss << total_operator_count << ((total_operator_count == 1) ? " operator" : " operators")
           << " in total.\n";

        return ss.str();
    }
}