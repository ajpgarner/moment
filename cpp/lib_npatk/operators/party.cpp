/**
 * party.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "party.h"
#include "context.h"
#include "utilities/alphabetic_namer.h"

#include <limits>
#include <iostream>

namespace NPATK {
    Party::Party(party_name_t id, std::string named, oper_name_t num_opers, Operator::Flags default_flags)
            : party_id{id}, id{this->party_id}, Measurements(*this), name{std::move(named)} {

        this->operators.reserve(num_opers);
        for (oper_name_t o = 0; o < num_opers; ++o) {
            this->operators.emplace_back(o, this->party_id, default_flags);
            this->operator_to_measurement.emplace_back(std::numeric_limits<size_t>::max());
        }
    }

    Party::Party(party_name_t id, oper_name_t num_opers, Operator::Flags default_flags)
        : Party{id, AlphabeticNamer::index_to_name(id, true), num_opers, default_flags} {  }


    void Party::set_offsets(party_name_t new_id, mmt_name_t new_mmt_offset, bool force_refresh) noexcept {
        if (force_refresh || (new_id != this->party_id) || (new_mmt_offset != this->global_mmt_offset)) {
            this->party_id = new_id;
            this->global_mmt_offset = new_mmt_offset;
            for (auto &mmt: this->measurements) {
                mmt.index.party = new_id;
                mmt.index.global_mmt = static_cast<mmt_name_t>(this->global_mmt_offset + mmt.index.mmt);
            }
        }
    }

    void Party::add_measurement(Measurement mmt, bool defer_recount) {
        // Measurement must have one outcome
        assert(mmt.num_outcomes >= 1);

        // Register measurement in list...
        this->measurements.emplace_back(std::move(mmt));
        const auto mmt_no = static_cast<mmt_name_t>(this->measurements.size()-1);
        auto& measurement = this->measurements.back();
        measurement.offset = this->operators.size();
        measurement.index.party = this->party_id;
        measurement.index.mmt = mmt_no;
        measurement.index.global_mmt = static_cast<mmt_name_t>(this->global_mmt_offset + mmt_no);

        // Determine operator flags and number
        const size_t operators_added = measurement.num_operators();
        const size_t init_id = this->operators.size();
        this->operators.reserve(init_id + operators_added);
        Operator::Flags oFlags = measurement.projective ? Operator::Flags::Idempotent : Operator::Flags::None;
        size_t final_id = init_id + operators_added;

        // Create operators, register them with measurement.
        for (size_t index = init_id; index < final_id; ++index) {
            this->operators.emplace_back(index, this->party_id, oFlags);
            this->operator_to_measurement.emplace_back(mmt_no);
        }
        assert(this->operators.size() == this->operator_to_measurement.size());

        // Register mutual exclusion
        if (measurement.projective) {
            for (size_t l_index = init_id; l_index < final_id; ++l_index) {
                for (size_t r_index = l_index + 1; r_index < final_id; ++r_index) {
                    this->add_mutex(static_cast<oper_name_t>(l_index), static_cast<oper_name_t>(r_index));
                }
            }
        }

        // If we have a context, recount operators, measurements, etc.
        if (!defer_recount && (this->context != nullptr)) {
            this->context->reenumerate();
        }
    }

    std::ostream& Party::format_operator(std::ostream& os, const Operator &op) const {
        assert(op.party == this->party_id);
        assert(op.id < this->operators.size());

        size_t mmt_id = this->operator_to_measurement[op.id];
        if (mmt_id != std::numeric_limits<size_t>::max()) {
            const auto& mmt = this->measurements[mmt_id];
            os << mmt.name << std::to_string(op.id - mmt.offset);
        } else {
            os << op.id;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const Party &the_party) {
        os << the_party.name << ": ";
        if (the_party.operators.empty()) {
        //if (the_party.measurements.empty()) {
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
            for (auto oper_index = mmt.get_offset(), oper_index_max = mmt.get_offset() + mmt.num_operators();
                oper_index < oper_index_max; ++oper_index) {
                if (one_elem) {
                    os << ", ";
                }
                the_party.format_operator(os, the_party.operators[oper_index]);
                one_elem = true;
            }
            if (mmt.complete) {
                if (one_elem) {
                    os << ", ";
                }
                os << "("
                   << mmt.name << std::to_string(mmt.num_operators())
                   << ")";
            }
            os << "}";
            one_measurement = true;
        }

        // Then, spare operators if any
        bool one_loose_oper = false;
        for (size_t oper_index = 0; oper_index < the_party.operators.size(); ++oper_index ) {
            if (the_party.operator_to_measurement[oper_index] != std::numeric_limits<size_t>::max()) {
                continue;
            }
            if (one_loose_oper) {
                os << ", ";
            } else {
                if (one_measurement) {
                    os << ", ";
                    one_measurement = true;
                }
                os << "{";
            }
            the_party.format_operator(os, the_party.operators[oper_index]);
            one_loose_oper = true;
        }
        if (one_loose_oper) {
            os << "}";
        }

        return os;
    }

    std::vector<Party>
    Party::MakeList(party_name_t num_parties, oper_name_t mmts_per_party, oper_name_t outcomes_per_mmt,
                    bool projective) {
        std::vector<Party> output;
        output.reserve(num_parties);

        AlphabeticNamer party_namer{true};
        AlphabeticNamer mmt_namer{false};

        for (party_name_t p = 0; p < num_parties; ++p) {
            output.emplace_back(p, party_namer(p), 0);
            Party& lastParty = output.back();
            for (oper_name_t o = 0; o < mmts_per_party; ++o) {
                lastParty.add_measurement(Measurement(mmt_namer(o), outcomes_per_mmt, projective));
            }
        }
        return output;
    }


    std::vector<Party>
    Party::MakeList(party_name_t num_parties, oper_name_t opers_per_party, Operator::Flags default_flags) {
        std::vector<Party> output;
        output.reserve(num_parties);

        for (party_name_t p = 0; p < num_parties; ++p) {
            output.emplace_back(p, opers_per_party, default_flags);
        }
        return output;
    }

    std::vector<Party>
    Party::MakeList(const std::vector<size_t>& operators_per_party_list, Operator::Flags default_flags) {
        std::vector<Party> output;
        output.reserve(operators_per_party_list.size());

        party_name_t p = 0;
        for (auto count : operators_per_party_list) {
            output.emplace_back(p, count, default_flags);
            ++p;
        }
        return output;
    }

    std::vector<Party> Party::MakeList(const std::vector<size_t>& mmts_per_party,
                                       const std::vector<size_t>& outcomes_per_mmt) {
        const auto num_parties = static_cast<party_name_t>(mmts_per_party.size());
        std::vector<Party> output;
        output.reserve(num_parties);

        AlphabeticNamer party_namer{true};
        AlphabeticNamer mmt_namer{false};

        auto mmtCountIter = mmts_per_party.cbegin();
        auto outcomePerMmtIter = outcomes_per_mmt.cbegin();

        for (party_name_t p = 0; p < num_parties; ++p) {
            output.emplace_back(p, party_namer(p), 0);
            Party& lastParty = output.back();
            auto num_mmts = *mmtCountIter;
            for (oper_name_t o = 0; o < num_mmts; ++o) {
                lastParty.add_measurement(Measurement(mmt_namer(o), *outcomePerMmtIter, true));
                ++outcomePerMmtIter;
            }
            ++mmtCountIter;
        }
        return output;
    }
}