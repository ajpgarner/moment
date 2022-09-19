/**
 * party.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"
#include "measurement.h"

#include <cassert>

#include <iterator>
#include <set>
#include <string>
#include <vector>
#include <iosfwd>

namespace NPATK {

    class Context;

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
        const party_name_t& id;
        std::string name;

    private:
        mmt_name_t global_mmt_offset = 0;
        size_t global_operator_offset = 0;

        std::vector<Operator> operators{};
        std::set<std::pair<oper_name_t, oper_name_t>> mutex{};
        Context * context = nullptr;

        std::vector<Measurement> measurements{};
        std::vector<size_t> operator_to_measurement{};

    public:
        Party(const Party& rhs) = delete;

        Party(Party&& rhs) noexcept :
                party_id{rhs.party_id}, id{this->party_id},
                Measurements(*this), name{std::move(rhs.name)},
                global_mmt_offset(rhs.global_mmt_offset), global_operator_offset(rhs.global_operator_offset),
                operators(std::move(rhs.operators)), mutex(std::move(rhs.mutex)), context{rhs.context},
                measurements(std::move(rhs.measurements)), operator_to_measurement(std::move(rhs.operator_to_measurement)) {}

        Party(party_name_t id, std::string named)
            : Measurements(*this), party_id{id}, id{this->party_id}, name{std::move(named)} { }

        Party(party_name_t id, std::string named, oper_name_t num_opers,
              Operator::Flags default_flags = Operator::Flags::None);

        explicit Party(party_name_t id, oper_name_t num_opers,
                       Operator::Flags default_flags = Operator::Flags::None);

        [[nodiscard]] constexpr auto begin() const noexcept { return this->operators.begin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->operators.end(); }

        [[nodiscard]] constexpr Operator& operator[](size_t index)  noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }

        [[nodiscard]] constexpr const Operator& operator[](size_t index) const noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }

        [[nodiscard]] constexpr const Operator&
        measurement_outcome(size_t mmt_index, size_t outcome_index) const noexcept {
            assert(mmt_index < this->measurements.size());
            const auto& mmt = this->measurements[mmt_index];
            assert(outcome_index < mmt.num_operators());
            return this->operators[mmt.offset + outcome_index];
        }

        void add_measurement(Measurement mmt, bool defer_recount = false);

        void set_offsets(party_name_t new_id, mmt_name_t new_mmt_offset, bool force_refresh = false) noexcept;

        std::ostream& format_operator(std::ostream&, const Operator& op) const;

        /**
         * Low-level command to register two operators A & B as being mutually exclusive such that AB = 0.
         * @param lhs_id Operator A
         * @param rhs_id Operator B
         */
        void add_mutex(oper_name_t lhs_id, oper_name_t rhs_id) {
            if (lhs_id < rhs_id) {
                this->mutex.emplace(std::make_pair(lhs_id, rhs_id));
            } else {
                this->mutex.emplace(std::make_pair(rhs_id, lhs_id));
            }
        }

        /**
         * Test if a string of two operators AB is identically zero, because the operators are mutually exclusive.
         * @param lhs_id Operator A
         * @param rhs_id Operator B
         * @return True if AB evaluates to zero
         */
        [[nodiscard]] bool exclusive(oper_name_t lhs_id, oper_name_t rhs_id) const noexcept {
            return mutex.contains((lhs_id < rhs_id) ? std::make_pair(lhs_id, rhs_id) : std::make_pair(rhs_id, lhs_id));
        };

        /**
         * @return The total number of operators associated with this party.
         */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        static std::vector<Party> MakeList(party_name_t num_parties,
                                           oper_name_t mmts_per_party,
                                           oper_name_t outcomes_per_mmt,
                                           bool projective = true);

        static std::vector<Party> MakeList(party_name_t num_parties, oper_name_t opers_per_party,
                                           Operator::Flags default_flags = Operator::Flags::None);

        static std::vector<Party> MakeList(std::initializer_list<oper_name_t> operators_per_party_list,
                                           Operator::Flags default_flags = Operator::Flags::None);

        friend std::ostream& operator<< (std::ostream& os, const Party& the_party);

        friend class Context;
    };


}