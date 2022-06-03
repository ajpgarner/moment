/*
 * (c) 2022-2022 Austrian Academy of Sciences.
 *
 * NPAToolKit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#pragma once

#include "operator.h"

#include <cassert>

#include <vector>
#include <iterator>
#include <iosfwd>

namespace NPATK {

    class OperatorCollection;

    class OperatorSequence {
    private:
        std::vector<Operator> constituents{};
        OperatorCollection * context = nullptr;

    public:
        constexpr OperatorSequence() = default;

        OperatorSequence(std::initializer_list<Operator> operators)
            : constituents(operators) {
            this->to_canonical_form();
        }

        explicit OperatorSequence(std::vector<Operator>&& operators)
                : constituents(std::move(operators)) {
            this->to_canonical_form();
        }

        constexpr OperatorSequence(const OperatorSequence& rhs) = default;

        constexpr OperatorSequence(OperatorSequence&& rhs) noexcept = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        [[nodiscard]] constexpr auto begin() const noexcept { return this->constituents.cbegin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->constituents.cend(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->constituents.empty(); }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->constituents.size(); }

        [[nodiscard]] constexpr const Operator& operator[](size_t i) const noexcept {
            assert(i < this->constituents.size());
            return this->constituents[i];
        }

        constexpr bool operator==(const OperatorSequence& rhs) const noexcept {
            if (this->constituents.size() != rhs.constituents.size()) {
                return false;
            }
            for (size_t i = 0, iMax = this->constituents.size(); i < iMax; ++i) {
                if (this->constituents[i] != rhs.constituents[i]) {
                    return false;
                }
            }
            return true;
        }

        friend std::ostream& operator<<(std::ostream& os, const OperatorSequence& seq);


        template<std::input_iterator iter_t>
        inline OperatorSequence& append(iter_t begin, iter_t end) {
            this->constituents.reserve(this->constituents.size() + std::distance(begin, end));
            this->constituents.insert(this->constituents.end(), begin, end);
            this->to_canonical_form();
            return *this;
        }

        inline OperatorSequence& append(std::initializer_list<Operator> opList) {
            return this->append(opList.begin(), opList.end());
        }

        OperatorSequence& operator *= (const OperatorSequence& rhs) {
            return this->append(rhs.constituents.begin(), rhs.constituents.end());
        }


        inline friend OperatorSequence operator * (const OperatorSequence& lhs, const OperatorSequence& rhs) {
            OperatorSequence output{lhs};
            output *= rhs;
            return output;
        }

        inline friend OperatorSequence operator * (OperatorSequence&& lhs, const OperatorSequence& rhs) {
            lhs *= rhs;
            return lhs;
        }


    private:
        void to_canonical_form() noexcept;
    };
}