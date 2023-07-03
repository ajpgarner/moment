/**
 * party.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "party.h"
#include "locality_context.h"
#include "locality_operator_formatter.h"
#include "utilities/alphabetic_namer.h"

#include <limits>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Moment::Locality {
   Party::Party(party_name_t id, std::string the_name, std::vector<Measurement> &&mmt_in)
        : party_id{id}, Measurements{*this}, name{std::move(the_name)}, measurements{std::move(mmt_in)} {

        // Set up measurements
        this->party_operator_count = 0;
        mmt_name_t mmt_id = 0;
        for (auto& mmt : this->measurements) {
            mmt.party_offset = this->party_operator_count;
            mmt.index.mmt = mmt_id;
            this->party_operator_count += mmt.num_operators();
            ++mmt_id;
        }

        // Set up party-index-to-mmt aliases
        this->offset_id_to_local_mmt.reserve(this->party_operator_count);
        for (const auto& mmt : this->measurements) {
            std::fill_n(std::back_inserter(this->offset_id_to_local_mmt), mmt.num_operators(), mmt.index.mmt);
        }

        // Note operator IDs included within party
        this->included_operators.reserve(this->party_operator_count);
        const oper_name_t iMax = this->global_operator_offset + this->party_operator_count;
        for (oper_name_t i = this->global_operator_offset; i < iMax; ++i) {
            this->included_operators.emplace_back(i);
        }

   }

    Party::Party(party_name_t id, std::vector<Measurement>&& measurements)
        : Party{id, AlphabeticNamer::index_to_name(id, true), std::move(measurements)} {
    }



    std::ostream &operator<<(std::ostream &os, const Party &the_party) {
        // Get formatting object
        const static NaturalLOFormatter default_formatter{};

        os << the_party.name << ": ";

        if (the_party.measurements.empty()) {
            os << " [empty]";
            return os;
        }

        // First, operators in measurements
        bool one_measurement = false;
        for (const auto& mmt : the_party.measurements) {
            if (one_measurement) {
                os << ", ";
            }
            os << "{";
            bool one_elem = false;

            // Normal (explicitly defined) operators
            for (size_t oper_index = 0; oper_index < mmt.num_operators(); ++oper_index) {
                if (one_elem) {
                    os << ", ";
                }
                default_formatter.format(os, mmt, static_cast<oper_name_t>(oper_index));
                // os << mmt.name << oper_index;
                one_elem = true;
            }

            // Implicitly defined operator
            if (one_elem) {
                os << ", ";
            }
            os << "(";
            default_formatter.format(os, mmt, mmt.num_operators());
            os << ")";

            os << "}";
            one_measurement = true;
        }


        return os;
    }
    /**
     * Gets the name of this operator (if within party)
     */
    std::string Party::format_operator(const LocalityOperatorFormatter& formatter, oper_name_t op) const {
        std::stringstream ss;
        this->format_operator(ss, formatter, op);
        return ss.str();
    }

    /**
     * Gets the name of this operator (if within party)
     */
    std::ostream& Party::format_operator(std::ostream& os, const LocalityOperatorFormatter& formatter, oper_name_t op) const {
        auto op_local_index = op - this->global_operator_offset;
        if ((op_local_index < 0) || (op_local_index >= this->party_operator_count)) {
            os << "[UNK#" << op << "]";
            return os;
        }

        auto mmt_id = this->offset_id_to_local_mmt[op_local_index];
        const auto& mmt = this->measurements[mmt_id];
        auto outcome_num = op_local_index - mmt.party_offset;

        formatter.format(os, *this, mmt, static_cast<oper_name_t>(outcome_num));

        return os;
    }

    void Party::set_offsets(party_name_t new_id, oper_name_t new_oper_offset, mmt_name_t new_mmt_offset) noexcept {
        // Set IDs and offsets
        this->party_id = new_id;
        this->global_operator_offset = new_oper_offset;
        this->global_measurement_offset = new_mmt_offset;

        // Propagate IDs and offsets to measurements
        for (auto &mmt: this->measurements) {
            mmt.index.party = this->party_id;
            mmt.index.global_mmt = static_cast<mmt_name_t>(this->global_measurement_offset + mmt.index.mmt);
        }

        // Propagate offsets to included operators
        this->included_operators.clear();
        this->included_operators.reserve(this->party_operator_count);
        const oper_name_t iMax = this->global_operator_offset + this->party_operator_count;
        for (oper_name_t i = this->global_operator_offset; i < iMax; ++i) {
            this->included_operators.emplace_back(i);
        }
    }


    std::span<const oper_name_t> Party::operators() const {
        if (!context) {
            throw std::logic_error{"Cannot access operators of party until party has been attached to a context."};
        }

        return std::span<const oper_name_t>{
                this->included_operators.begin(), this->included_operators.end()
        };
    }


    const Measurement& Party::measurement_of(oper_name_t op) const {
        if (!this->context) {
            throw std::logic_error{"Cannot access operators of party until party has been attached to a context."};
        }
        auto op_local_index = op - this->global_operator_offset;
        if ((op_local_index < 0) || (op_local_index >= this->party_operator_count)) {
            throw std::range_error{"Operator index out of range."};
        }
        auto mmt_id = this->offset_id_to_local_mmt[op_local_index];
        return this->measurements[mmt_id];
    }

    oper_name_t Party::measurement_outcome(size_t mmt_index, size_t outcome_index) const {
        if (!context) {
            throw std::logic_error{"Cannot access operators of party until party has been attached to a context."};
        }
        if (mmt_index >= this->measurements.size()) {
            throw std::range_error{"Measurement index out of range."};
        }
        const auto& mmt = this->measurements[mmt_index];
        if (outcome_index >= mmt.num_operators()) {
            throw std::range_error{"Outcome index out of range."};
        }

        auto operIndex = this->global_operator_offset + mmt.party_offset + outcome_index;
        return operIndex;
    }

    oper_name_t Party::operator[](size_t index) const {
        if (index >= this->party_operator_count) {
            throw std::range_error{"Operator index out of range."};
        }
        return this->included_operators[index];
    }

    bool Party::mutually_exclusive(const oper_name_t lhs, const oper_name_t rhs) const noexcept {
        // X^2 != 0 in general.
        if (lhs == rhs) {
            return false;
        }

        return (this->offset_id_to_local_mmt[lhs - this->global_operator_offset]
                 == this->offset_id_to_local_mmt[rhs - this->global_operator_offset]);
    }


    std::vector<Party>
    Party::MakeList(party_name_t num_parties,
                    mmt_name_t mmts_per_party,
                    oper_name_t outcomes_per_mmt) {
        std::vector<Party> output;
        output.reserve(num_parties);

        AlphabeticNamer party_namer{true};
        AlphabeticNamer mmt_namer{false};

        for (party_name_t p = 0; p < num_parties; ++p) {
            // Make measurement list
            std::vector<Measurement> mmtList;
            mmtList.reserve(mmts_per_party);
            for (mmt_name_t mId = 0; mId < mmts_per_party; ++mId) {
                mmtList.emplace_back(mmt_namer(mId), outcomes_per_mmt);
            }

            output.emplace_back(p, party_namer(p),std::move(mmtList));
        }
        return output;
    }


    std::vector<Party>
    Party::MakeList(const std::vector<size_t>& mmts_per_party,
                    const std::vector<size_t>& outcomes_per_mmt) {
        const auto num_parties = static_cast<party_name_t>(mmts_per_party.size());
        std::vector<Party> output;
        output.reserve(num_parties);

        AlphabeticNamer party_namer{true};
        AlphabeticNamer mmt_namer{false};

        auto mmtCountIter = mmts_per_party.cbegin();
        auto outcomePerMmtIter = outcomes_per_mmt.cbegin();

        for (party_name_t p = 0; p < num_parties; ++p) {
            assert(mmtCountIter != mmts_per_party.cend());

            std::vector<Measurement> mmtList;

            auto num_mmts = *mmtCountIter;
            for (oper_name_t o = 0; o < num_mmts; ++o) {
                assert(outcomePerMmtIter != outcomes_per_mmt.cend());
                mmtList.emplace_back(mmt_namer(o), static_cast<oper_name_t>(*outcomePerMmtIter));
                ++outcomePerMmtIter;
            }

            output.emplace_back(p, party_namer(p), std::move(mmtList));

            ++mmtCountIter;
        }
        return output;
    }


}