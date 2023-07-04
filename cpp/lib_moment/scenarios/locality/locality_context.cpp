/**
 * locality_context.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_context.h"

#include "dictionary/operator_sequence.h"

#include "locality_operator_formatter.h"
#include "locality_osg.h"

#include "party.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace Moment::Locality {
    namespace {
          size_t count_operators(const std::vector<Party>& parties) {
            size_t val = 0;
            for (const auto& party : parties) {
                val += party.size();
            }
            return val;
        }
    };

    LocalityContext::LocalityContext() : Context{0}, Parties{*this} {
    }

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
                    this->global_to_local_indices.emplace_back(party_index, mmt_index, 
                                                               static_cast<uint32_t>(oper_index));
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

        assert(this->global_op_id_to_party.size() == this->operator_count);
    }

    bool LocalityContext::additional_simplification(sequence_storage_t &op_sequence, bool& negated) const {
        // Do nothing on empty set
        if (op_sequence.empty()) {
            return false;
        }

        // Verify operator sequence is valid
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operator_count)) {
                throw std::range_error{"Operator ID higher than number of known operators."};
            }
        }

        // Group first by party (preserving ordering within each party)
        std::stable_sort(op_sequence.begin(), op_sequence.end(),
                         [&](const auto& lhs, const auto& rhs) {
            return this->global_op_id_to_party[lhs] < this->global_op_id_to_party[rhs];
        });

        // Remove excess idempotent elements [all ops are projectors in this context!]
        auto trim_idem = std::unique(op_sequence.begin(), op_sequence.end());
        op_sequence.erase(trim_idem, op_sequence.end());

        // Look for mutually exclusive operators
        auto lhs_iter = op_sequence.begin();
        auto rhs_iter = op_sequence.begin() + 1;

        while (rhs_iter != op_sequence.end()) {
            // Only do comparison if operators are in same party, and party is well defined...
            const auto lhs_party_id = this->global_op_id_to_party[*lhs_iter];
            const auto rhs_party_id = this->global_op_id_to_party[*rhs_iter];

            if (lhs_party_id == rhs_party_id) {
                assert((lhs_party_id >= 0) && (lhs_party_id < this->parties.size()));
                const auto& party = this->parties[lhs_party_id];
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
        assert (global_index < this->global_mmt_id_to_party.size());
        auto party_id = this->global_mmt_id_to_party[global_index];
        auto mmt_id = static_cast<mmt_name_t>(global_index - this->parties[party_id].global_measurement_offset);

        assert(mmt_id >= 0);
        return PMIndex{party_id, mmt_id, global_index};
    }

    void LocalityContext::populate_global_mmt_index(std::span<PMIndex> pm_index) const noexcept {
        for (auto& pm : pm_index) {
            assert (pm.party < this->parties.size());
            assert (pm.mmt < this->parties[pm.party].measurements.size());
            pm.global_mmt = static_cast<mmt_name_t>(this->parties[pm.party].global_measurement_offset + pm.mmt);
        }
    }

    std::vector<size_t> LocalityContext::PM_to_global_index(std::span<const PMIndex> pm_index) const {
        std::vector<size_t> output{};
        output.reserve(pm_index.size());
        for (const auto& pm : pm_index) {
            if (pm.party >= this->parties.size()) {
                std::stringstream errSS;
                errSS << "Party " << pm.party << " out of range.";
                throw std::range_error(errSS.str());
            }
            const auto& party = this->parties[pm.party];
            if (pm.mmt >= party.measurements.size()) {
                std::stringstream errSS;
                errSS << "Measurement " << pm.mmt << " out of range for party \"" << party.name << "\".";
                throw std::range_error(errSS.str());
            }
            output.emplace_back(party.global_measurement_offset + pm.mmt);
        }
        return output;
    }

    std::vector<size_t> LocalityContext::outcomes_per_measurement(const std::span<const PMIndex> indices) const {
        std::vector<size_t> output;
        output.reserve(indices.size());

        for (auto index : indices) {
            if (index.party >= this->parties.size()) {
                std::stringstream errSS;
                errSS << "Party " << index.party << " out of range.";
                throw std::range_error(errSS.str());
            }
            const auto& party = this->Parties[index.party];

            if (index.mmt >= party.measurements.size()) {
                std::stringstream errSS;
                errSS << "Measurement " << index.mmt << " out of range for party \"" << party.name << "\".";
                throw std::range_error(errSS.str());
            }
            const auto& mmt = party.Measurements[index.mmt];
            output.push_back(mmt.num_outcomes);
        }

        return output;
    }


    std::vector<size_t> LocalityContext::outcomes_per_measurement() const {
        std::vector<size_t> output;
        output.reserve(this->total_measurement_count);
        for (auto& party : this->parties) {
            for (auto& mmt : party.measurements) {
                output.emplace_back(mmt.num_outcomes);
            }
        }
        return output;
    }

    std::vector<size_t> LocalityContext::outcomes_per_party() const {
        std::vector<size_t> output;
        output.reserve(this->parties.size());
        for (auto& party : this->parties) {
            size_t party_outcomes = 0;
            for (auto& mmt : party.measurements) {
                party_outcomes += mmt.num_outcomes;

            }
            output.emplace_back(party_outcomes);
        }
        return output;
    }

    std::string LocalityContext::format_sequence(const OperatorSequence &seq) const {
        NaturalLOFormatter formatter;
        return this->format_sequence(formatter, seq);
    }

    std::string LocalityContext::format_sequence(const LocalityOperatorFormatter& formatter,
                                                 const OperatorSequence &seq) const {
        if (seq.zero()) {
            return "0";
        }
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;
        if (seq.negated()) {
            ss << "-";
        }
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
                party.format_operator(ss, formatter, oper);
            }
        }
        return ss.str();
    }

    std::string LocalityContext::format_sequence(const LocalityOperatorFormatter& formatter,
                                                 const std::span<const PMOIndex> indices, const bool zero) const {
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

            formatter.format(ss, party, mmt, static_cast<oper_name_t>(index.outcome));
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

        ss << this->operator_count << ((this->operator_count == 1) ? " operator" : " operators")
           << " in total.\n";

        return ss.str();
    }

    std::unique_ptr<OperatorSequenceGenerator> LocalityContext::new_osg(const size_t word_length) const {
        return std::make_unique<LocalityOperatorSequenceGenerator>(*this, word_length);
    }



}