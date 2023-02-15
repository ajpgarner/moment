/**
 * linear_combo.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

namespace Moment {
    template<typename index_t = size_t, typename weight_t = double>
    class LinearCombo {
    public:
        using column_value_t = std::pair<index_t, weight_t>;
        using data_t = std::vector<column_value_t>;
        using map_t = std::map<index_t, weight_t>;
    private:
        data_t data;

    public:

        constexpr LinearCombo() = default;

        constexpr explicit LinearCombo(data_t input) : data(std::move(input)) {
            std::sort(data.begin(), data.end(),
                      [](const column_value_t& lhs, const column_value_t& rhs) { return lhs.first < rhs.first;} );
        }

        constexpr explicit LinearCombo(const map_t& input)
            : data(input.cbegin(), input.cend()) {
        }

        constexpr LinearCombo(std::initializer_list<column_value_t> input)
            : LinearCombo(std::move(data_t(std::move(input)))) { }

        friend constexpr void swap(LinearCombo& lhs, LinearCombo& rhs) noexcept {
            std::swap(lhs.data, rhs.data);
        }

        [[nodiscard]] size_t size() const noexcept { return this->data.size(); }
        [[nodiscard]] bool empty() const noexcept { return this->data.empty(); }
        [[nodiscard]] auto begin() const noexcept { return this->data.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->data.cend(); }
        [[nodiscard]] const column_value_t& operator[](size_t index) const noexcept { return this->data[index]; }

        [[nodiscard]] friend constexpr LinearCombo operator+(const LinearCombo& lhs, const LinearCombo& rhs) {
            // Get data iterators
            auto lhsIter = lhs.data.begin();
            const auto lhsEnd = lhs.data.end();
            if (lhsIter == lhsEnd) {
                return rhs; // LHS is empty, copy RHS
            }
            auto rhsIter = rhs.data.begin();
            const auto rhsEnd = rhs.data.end();
            if (rhsIter == rhsEnd) {
                return lhs; // RHS is empty, copy LHS
            }

            // Copy and merge, maintaining ordering
            LinearCombo output;
            // iterate until both iterators are at end
            while ((lhsIter != lhsEnd) || (rhsIter != rhsEnd)) {
                if ((rhsIter == rhsEnd) || ((lhsIter != lhsEnd) && (lhsIter->first < rhsIter->first))) {
                    output.data.push_back(*lhsIter); // Copy element from LHS
                    ++lhsIter;
                } else if ((lhsIter == lhsEnd) || ((rhsIter != rhsEnd) && (lhsIter->first > rhsIter->first))) {
                    output.data.push_back(*rhsIter); // Copy element from RHS
                    ++rhsIter;
                } else {
                    assert(lhsIter != lhsEnd);
                    assert(rhsIter != rhsEnd);
                    assert(lhsIter->first == rhsIter->first);

                    const auto sumVals = lhsIter->second + rhsIter->second;
                    if (sumVals != 0) {
                        output.data.emplace_back(lhsIter->first, sumVals);
                    }
                    ++lhsIter;
                    ++rhsIter;
                }
            }
            return output;
        }

        constexpr LinearCombo& operator +=(const LinearCombo& rhs) {
            *this = LinearCombo{*this + rhs};
            return *this;
        }

        constexpr LinearCombo& operator *=(const weight_t factor) {
            for (auto& entry : this->data) {
                entry.second *= factor;
            }
            return *this;
        }

        [[nodiscard]] friend LinearCombo operator*(const LinearCombo& lhs, const weight_t factor) {
            LinearCombo output{lhs};
            output *= factor;
            return output;
        }

        constexpr bool operator==(const LinearCombo& rhs) const noexcept {
            if (this->data.size() != rhs.data.size()) {
                return false;
            }
            for (size_t index = 0; index < this->data.size(); ++index) {
                if ((this->data[index].first != rhs.data[index].first)
                    || (this->data[index].second != rhs.data[index].second)) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const LinearCombo& rhs) const noexcept {
            return !(this->operator==(rhs));
        }


    };

    using SymbolCombo = LinearCombo<symbol_name_t, double>;
}