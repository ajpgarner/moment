/**
 * locality_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "locality_context.h"

#include "../operator_sequence.h"

#include "party.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

namespace NPATK {
    namespace {
        struct LocalityOperator {
        public:
            oper_name_t id = 0;
            party_name_t party = 0;

            LocalityOperator() = default;
            LocalityOperator(oper_name_t i, party_name_t p) : id{i}, party{p} { }

            /**
            * Predicate: true if the party of LHS is less than that of RHS.
            */
            struct PartyComparator {
                constexpr bool operator()(const LocalityOperator &lhs, const LocalityOperator &rhs) const noexcept {
                    return lhs.party < rhs.party;
                }
            };

            /**
             * Predicate: true if lhs = rhs, and lhs is idempotent.
             * I.e., true if 'AB' can be replaced by 'A'.
             */
            struct IsRedundant {
                constexpr bool operator()(const LocalityOperator &lhs, const LocalityOperator &rhs) const noexcept {
                    return (lhs.id == rhs.id);
                }
            };

        };

        size_t count_operators(const std::vector<Party>& parties) {
            size_t val = 0;
            for (const auto& party : parties) {
                val += party.size();
            }
            return val;
        }
    };

    LocalityContext::LocalityContext(std::vector<Party> &&in_party) noexcept
        : Context{count_operators(in_party)}, Parties{*this},
          parties{std::move(in_party)}, total_measurement_count{0} {
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
            this->global_mmt_id_to_party.insert(this->global_mmt_id_to_party.end(), party.measurements.size(), party_index);

            // Register operators...
            size_t party_op_count = 0;
            mmt_name_t mmt_index = 0;
            for (const auto& mmt : party.measurements) {
                for (size_t oper_index = 0; oper_index < mmt.num_operators(); ++oper_index) {
                    //this->operators.emplace_back(total_operator_count);
                    this->global_to_local_indices.emplace_back(party_index, mmt_index, oper_index);
                    ++party_op_count;
                    ++total_operator_count;
                }
                ++mmt_index;
            }
            this->global_op_id_to_party.insert(this->global_op_id_to_party.end(),
                                               party_op_count, party_index);

            this->mmts_per_party.emplace_back(party.measurements.size());
            this->ops_per_party.emplace_back(party.party_operator_count);

            ++party_index;
        }

        assert(this->global_op_id_to_party.size() == this->operators.size());
    }



    bool LocalityContext::additional_simplification(std::vector<oper_name_t>& op_sequence) const {
        // Do nothing on empty set
        if (op_sequence.empty()) {
            return false;
        }

        // Commutation between parties...
        std::vector<LocalityOperator> lo_seq;
        lo_seq.reserve(op_sequence.size());
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operators.size())) {
                throw std::range_error{"Operator ID higher than number of known operators."};
            }
            lo_seq.emplace_back(op, this->global_op_id_to_party[op]);
        }

        // Group first by party (preserving ordering within each party)
        std::stable_sort(lo_seq.begin(), lo_seq.end(),
                         LocalityOperator::PartyComparator{});

        // Remove excess idempotent elements.
        auto trim_idem = std::unique(lo_seq.begin(), lo_seq.end(),
                                     LocalityOperator::IsRedundant{});
        lo_seq.erase(trim_idem, lo_seq.end());


        // Look for mutually exclusive operators
        auto lhs_iter = lo_seq.begin();
        auto rhs_iter = lo_seq.begin() + 1;

        while (rhs_iter != lo_seq.end()) {
            // Only do comparison if operators are in same party, and party is well defined...
            if (lhs_iter->party == rhs_iter->party) {
                assert((lhs_iter->party >= 0) && (lhs_iter->party < this->parties.size()));
                const auto& party = this->parties[lhs_iter->party];
                // If mutually-exclusive operators are found next to each other, whole sequence is zero.
                if (party.mutually_exclusive(lhs_iter->id, rhs_iter->id)) {
                    op_sequence.clear();
                    return true;
                }
            }

            // Advance iterators
            lhs_iter = rhs_iter;
            ++rhs_iter;
        }

        // Copy transformed string to operator sequence
        op_sequence.clear();
        for (const auto& op : lo_seq) {
            op_sequence.emplace_back(op.id);
        }
        return false;
    }

    PMIndex LocalityContext::global_index_to_PM(size_t global_index) const noexcept {
        assert (global_index < this->global_mmt_id_to_party.size());
        auto party_id = this->global_mmt_id_to_party[global_index];
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
        bool done_once = false;
        for (const auto& oper : seq) {
            if (done_once) {
                ss << ";";
            } else {
                done_once = true;
            }

            if (oper > this->size()) {
                ss << "[UNK:" << oper << "]";
            } else {
                const auto &party = this->parties[this->global_op_id_to_party[oper]];
                ss << party.format_operator(oper);
            }
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