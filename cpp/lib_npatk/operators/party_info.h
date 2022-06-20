/**
 * party_info.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"

#include <cassert>

#include <iterator>
#include <set>
#include <string>
#include <vector>
#include <iosfwd>

namespace NPATK {

    class Context;

    struct Measurement {
    public:
        std::string name{};
        size_t num_outcomes = 0;
        bool projective = true;
        bool complete = true;

    protected:
        size_t offset = 0;
    public:
        constexpr Measurement() = default;
        constexpr Measurement(const Measurement&) = default;
        constexpr Measurement( Measurement&&) = default;

        constexpr Measurement(std::string name, size_t outcomes,
                              bool projective = true,
                              bool complete = true) noexcept
            : name{std::move(name)}, num_outcomes{outcomes},
              projective{projective}, complete{complete} {
            assert(outcomes >= 1);
        }

        [[nodiscard]] constexpr size_t get_offset() const noexcept { return this->offset; }

        [[nodiscard]] constexpr size_t num_operators() const noexcept {
            return this->num_outcomes - (this->complete ? 1 : 0);
        }

        friend class PartyInfo;
    };


    class PartyInfo : public Party {
    public:
        const std::string name;

    private:
        size_t global_offset = 0;
        std::vector<Operator> operators{};
        std::set<std::pair<oper_name_t, oper_name_t>> mutex{};
        Context * context = nullptr;

        std::vector<Measurement> measurements{};
        std::vector<size_t> operator_to_measurement{};

    public:
        PartyInfo(const PartyInfo& rhs) = delete;

        PartyInfo(PartyInfo&& rhs) = default;

        PartyInfo(party_name_t id, std::string named) : Party{id}, name{std::move(named)} { }

        PartyInfo(party_name_t id, std::string named, oper_name_t num_opers,
                  Operator::Flags default_flags = Operator::Flags::None);

        explicit PartyInfo(party_name_t id, oper_name_t num_opers,
                           Operator::Flags default_flags = Operator::Flags::None);




        [[nodiscard]] constexpr auto begin() const noexcept { return this->operators.begin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->operators.end(); }

        constexpr Operator& operator[](size_t index)  noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }

        constexpr const Operator& operator[](size_t index) const noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }



        void add_measurement(Measurement mmt);

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
        [[nodiscard]] constexpr bool exclusive(oper_name_t lhs_id, oper_name_t rhs_id) const noexcept {
            return mutex.contains((lhs_id < rhs_id) ? std::make_pair(lhs_id, rhs_id) : std::make_pair(rhs_id, lhs_id));
        };

        /**
         * @return The total number of operators associated with this party.
         */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        /**
         * @return The global offset of this party's operators in the global context.
         */
        [[nodiscard]] constexpr size_t offset() const noexcept { return this->global_offset; }


        static std::vector<PartyInfo> MakeList(party_name_t num_parties,
                                               oper_name_t mmts_per_party,
                                               oper_name_t outcomes_per_mmt,
                                               bool projective = true);

        static std::vector<PartyInfo> MakeList(party_name_t num_parties, oper_name_t opers_per_party,
                                               Operator::Flags default_flags = Operator::Flags::None);

        static std::vector<PartyInfo> MakeList(std::initializer_list<oper_name_t> operators_per_party_list,
                                                      Operator::Flags default_flags = Operator::Flags::None);

        friend std::ostream& operator<< (std::ostream& os, const PartyInfo& the_party);

        friend class Context;
    };
}