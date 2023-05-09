/**
 * party.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../context.h"
#include "measurement.h"
#include "integer_types.h"

#include <cassert>

#include <iterator>
#include <set>
#include <string>
#include <vector>
#include <iosfwd>

namespace Moment::Locality {

    class LocalityContext;
    class LocalityOperatorFormatter;

    class Party {
    public:
        class MeasurementRange {
            const Party& party;
        public:
            explicit MeasurementRange(const Party& p) noexcept : party{p} { }
            [[nodiscard]] auto begin() const noexcept { return party.measurements.begin(); }
            [[nodiscard]] auto end() const noexcept { return party.measurements.end(); }
            [[nodiscard]] auto size() const noexcept { return party.measurements.size(); }
            [[nodiscard]] auto empty() const noexcept { return party.measurements.empty(); }
            [[nodiscard]] inline const Measurement& operator[](size_t index) const noexcept {
                assert(index < party.measurements.size());
                return party.measurements[index];
            }
        } Measurements;

    private:
        party_name_t party_id = -1;

    public:
        const std::string name;

    private:
        std::vector<Measurement> measurements{};
        std::vector<mmt_name_t> offset_id_to_local_mmt{};

        mmt_name_t global_measurement_offset = 0;
        oper_name_t global_operator_offset = 0;
        oper_name_t party_operator_count = 0;

        std::vector<oper_name_t> included_operators;

        LocalityContext * context = nullptr;


    public:
        Party(const Party& rhs) = delete;

        Party(Party&& rhs) noexcept :
                Measurements(*this),
                party_id{rhs.party_id}, name{rhs.name},
                measurements(std::move(rhs.measurements)),
                offset_id_to_local_mmt{std::move(rhs.offset_id_to_local_mmt)},
                global_measurement_offset(rhs.global_measurement_offset),
                global_operator_offset(rhs.global_operator_offset),
                party_operator_count(rhs.party_operator_count),
                context{rhs.context} { }

        Party(party_name_t id, std::string the_name, std::vector<Measurement>&& measurements);

        Party(party_name_t id, std::vector<Measurement>&& measurements);

        [[nodiscard]] constexpr party_name_t id() const {return this->party_id; }

        /**
         * The index of the first operator in the party.
         */
        [[nodiscard]] constexpr oper_name_t  global_offset() const noexcept {
            return this->global_operator_offset;
        }

        /**
         * Gets a range of operators that correspond to the measurement outcomes from this party.
         * @return Span of operators
         * @throws logic_error if Party not yet attached to a Context.
         */
        [[nodiscard]] std::span<const oper_name_t> operators() const;

        /**
         * Gets an operator from this party.
         * @param index The index of the operator, relative to this party.
         * @return The requested opeartor
         * @throws logic_error if Party not yet attached to a Context.
         */
        [[nodiscard]] oper_name_t operator[](size_t index) const;

        /**
         * Gets the associated measurement from an operator in this party
         */
        [[nodiscard]] const Measurement& measurement_of(oper_name_t op) const;

        /**
         * Gets the name of this operator (if within party)
         */
        [[nodiscard]] std::string format_operator(const LocalityOperatorFormatter& formatter, oper_name_t op) const;

        /**
         * Gets the name of this operator (if within party)
         */
        std::ostream& format_operator(std::ostream& os, const LocalityOperatorFormatter& formatter,
                                      oper_name_t op) const;


        [[nodiscard]] auto begin() const {
            return this->included_operators.begin();
        }

        [[nodiscard]] auto end() const {
            return this->included_operators.end();
        }

        /**
         * Gets the operator corresponding to a particular outcome of a particular measurement in this party.
         * @param mmt_index The measurement index
         * @param outcome_index The outcome index
         * @return Reference to operator
         * @throws logic_error if Party not yet attached to a Context.
         */
        [[nodiscard]] oper_name_t measurement_outcome(size_t mmt_index, size_t outcome_index) const;

        /**
         * Test if a string of two operators AB is identically zero, because the operators are mutually exclusive.
         * @param lhs Operator A
         * @param rhs Operator B
         * @return True if AB evaluates to zero
         */
        [[nodiscard]] bool mutually_exclusive(oper_name_t lhs, oper_name_t rhs) const noexcept;

        /**
         * @return The total number of operators associated with this party.
         */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->party_operator_count; }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->party_operator_count <= 0; }


        static std::vector<Party> MakeList(party_name_t num_parties,
                                           mmt_name_t mmts_per_party,
                                           oper_name_t outcomes_per_mmt);

        static std::vector<Party> MakeList(const std::vector<size_t>& mmts_per_party,
                                           const std::vector<size_t>& outcomes_per_mmt);

        friend std::ostream& operator<< (std::ostream& os, const Party& the_party);

        friend class LocalityContext;

    private:
        void set_offsets(party_name_t new_id, oper_name_t new_oper_offset, mmt_name_t new_mmt_offset) noexcept;

    };


}