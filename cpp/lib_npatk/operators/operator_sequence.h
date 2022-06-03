/**
 * operator_sequence.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
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
        const OperatorCollection * context = nullptr;
        bool is_zero = false;
        bool is_hermitian = false;

    public:
        constexpr OperatorSequence() = default;

        OperatorSequence(std::initializer_list<Operator> operators)
            : constituents(operators) {
            this->to_canonical_form();
        }

        explicit OperatorSequence(std::vector<Operator>&& operators, const OperatorCollection * context = nullptr)
                : constituents(std::move(operators)), context{context} {
            this->to_canonical_form();
        }

        constexpr OperatorSequence(const OperatorSequence& rhs) = default;

        constexpr OperatorSequence(OperatorSequence&& rhs) noexcept = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        [[nodiscard]] constexpr auto begin() const noexcept { return this->constituents.cbegin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->constituents.cend(); }

        /**
         * True if no operators in sequence. Mathematically, this can be interpreted as either the identity operator
         *  if zero() returns false, or as the zero operator if zero() returns true.
         */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->constituents.empty(); }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->constituents.size(); }

        /**
         * True if the sequence represents zero.
         */
        [[nodiscard]] constexpr bool zero() const noexcept {
            return this->is_zero;
        }

        [[nodiscard]] constexpr const Operator& operator[](size_t i) const noexcept {
            assert(i < this->constituents.size());
            return this->constituents[i];
        }

        constexpr bool operator==(const OperatorSequence& rhs) const noexcept {
            if (this->constituents.size() != rhs.constituents.size()) {
                return false;
            }
            if (this->is_zero != rhs.is_zero) {
                return false;
            }
            for (size_t i = 0, iMax = this->constituents.size(); i < iMax; ++i) {
                if (this->constituents[i] != rhs.constituents[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const OperatorSequence& rhs) const noexcept {
            return !(*this == rhs);
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