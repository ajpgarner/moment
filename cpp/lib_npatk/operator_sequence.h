/**
 * hermitian_operator.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"

#include <vector>
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

        OperatorSequence(const OperatorSequence& rhs) = delete;

        OperatorSequence(OperatorSequence&& rhs) = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        [[nodiscard]] auto begin() const noexcept { return this->constituents.cbegin(); }

        [[nodiscard]] auto end() const noexcept { return this->constituents.cend(); }

        [[nodiscard]] bool empty() const noexcept { return this->constituents.empty(); }

        [[nodiscard]] size_t size() const noexcept { return this->constituents.size(); }

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

    private:
        void to_canonical_form() noexcept;
    };
}