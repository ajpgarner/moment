/**
 * hermitian_operator.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"

#include <vector>
#include <iterator>
#include <iosfwd>

namespace NPATK {

    struct Party {
        symbol_name_t id;
        constexpr explicit Party(symbol_name_t party_id) noexcept : id{party_id} { }

        constexpr bool operator<(const Party& rhs) const noexcept {
            return this->id < rhs.id;
        }
        constexpr bool operator==(const Party& rhs) const noexcept {
            return this->id == rhs.id;
        }
        constexpr bool operator!=(const Party& rhs) const noexcept {
            return this->id != rhs.id;
        }
    };

    struct Operator {
    public:
        /**
         * Predicate: true if the party ID of LHS is less than that of RHS.
         */
        struct PartyComparator {
            constexpr bool operator()(const Operator& lhs, const Operator& rhs) const noexcept {
                return lhs.party < rhs.party;
            }
        };

        /**
         * Predicate: true if lhs = rhs, and lhs is idempotent.
         * I.e., true if 'AB' can be replaced by 'A'.
         */
        struct IsRedundant {
            constexpr bool operator()(const Operator& lhs, const Operator& rhs) const noexcept {
                return lhs.idempotent && (lhs == rhs);
            }
        };

    public:
        symbol_name_t id;
        Party party;
        bool idempotent = false;

        constexpr Operator(symbol_name_t name, Party who, bool idem = false) noexcept
            : id{name}, party{who}, idempotent{idem} { }


        friend std::ostream& operator<<(std::ostream& os, const Operator& seq);

        constexpr bool operator==(const Operator& rhs) const noexcept {
            return (this->id == rhs.id) && (this->party == rhs.party);
        }
    };

    class OperatorSequence {
    private:
        std::vector<Operator> constituents{};

    public:
        constexpr OperatorSequence() = default;

        OperatorSequence(std::initializer_list<Operator> operators)
            : constituents(operators) {
            this->to_canonical_form();
        }

        constexpr OperatorSequence(const OperatorSequence& rhs) = default;

        constexpr OperatorSequence(OperatorSequence&& rhs) noexcept = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        [[nodiscard]] constexpr auto begin() const noexcept { return this->constituents.cbegin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->constituents.cend(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->constituents.empty(); }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->constituents.size(); }

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