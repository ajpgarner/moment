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


namespace NPATK {

    class PartyInfo : public Party {
    public:
        const std::string name;
    private:
        size_t global_offset = 0;
        std::vector<Operator> operators;
        std::set<std::pair<oper_name_t, oper_name_t>> mutex;

    public:
        PartyInfo(party_name_t id, std::string named, oper_name_t num_opers,
                  size_t global_offset = 0, Operator::Flags default_flags = Operator::Flags::None);

        explicit PartyInfo(party_name_t id, oper_name_t num_opers,
                           size_t global_offset = 0, Operator::Flags default_flags = Operator::Flags::None)
                : PartyInfo(id, std::to_string(id), num_opers, global_offset, default_flags) { }

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

        void add_mutex(oper_name_t lhs_id, oper_name_t rhs_id) {
            if (lhs_id < rhs_id) {
                this->mutex.emplace(std::make_pair(lhs_id, rhs_id));
            } else {
                this->mutex.emplace(std::make_pair(rhs_id, lhs_id));
            }
        }

        [[nodiscard]] constexpr bool exclusive(oper_name_t lhs_id, oper_name_t rhs_id) const noexcept {
            return mutex.contains((lhs_id < rhs_id) ? std::make_pair(lhs_id, rhs_id) : std::make_pair(rhs_id, lhs_id));
        };

        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        [[nodiscard]] constexpr size_t offset() const noexcept { return this->global_offset; }
    };
}