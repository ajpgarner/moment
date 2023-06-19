/**
 * symbol.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"
#include "dictionary/operator_sequence.h"

#include <optional>
#include <string>

namespace Moment {
    /**
     * Effectively represents a (monomial) moment.
     * Could associate an ID in the symbol table with an operator sequence.
     */
    class Symbol {
    private:
        symbol_name_t id = -1;
        std::optional<OperatorSequence> opSeq{};
        std::optional<OperatorSequence> conjSeq{};
        bool hermitian = false;
        bool antihermitian = false;
        ptrdiff_t real_index = -1;
        ptrdiff_t img_index = -1;
        std::string fwd_sequence_str;

    public:
        constexpr Symbol() = default;

        explicit Symbol(OperatorSequence sequence) :
                opSeq{std::move(sequence)},
                conjSeq{}, hermitian{true}, antihermitian{false},
                real_index{-1}, img_index{-1}, fwd_sequence_str{opSeq->formatted_string()} { }

        Symbol(OperatorSequence sequence, OperatorSequence conjSequence);

        /** True if a concrete operator sequence is associated with this symbol */
        [[nodiscard]] constexpr bool has_sequence() const noexcept { return this->opSeq.has_value(); }

        /** The symbol ID */
        [[nodiscard]] constexpr symbol_name_t Id() const noexcept { return this->id; }

        /**
         * The hash associated with the operator sequence.
         * Undefined behaviour if no operator sequence associated with this entry.
         */

        [[nodiscard]] constexpr size_t hash() const noexcept {
            assert(this->opSeq.has_value());
            return this->opSeq.value().hash();
        }

        /**
         * The hash associated with the operator sequence's complex conjugate..
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr size_t hash_conj() const noexcept {
            assert(this->conjSeq.has_value() || this->opSeq.has_value());
            return this->conjSeq.has_value() ? this->conjSeq->hash() : this->opSeq.value().hash();
        }

        /**
         * The operator sequence associated with this entry.
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept {
            assert(this->opSeq.has_value());
            return this->opSeq.value();
        }

        /**
         * The operator sequence associated with this entry's complex conjugate.
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
            assert(this->conjSeq.has_value() || this->opSeq.has_value());
            return this->hermitian ? opSeq.value() : this->conjSeq.value();
        }

        /**
         * Formatted view of sequence, if any, otherwise just symbol name.
         */
        [[nodiscard]] std::string formatted_sequence() const;

        /**
         * Formatted view of conjugate sequence, if any, otherwise just symbol name.
         */
        [[nodiscard]] std::string formatted_sequence_conj() const;

        /**
         * Does the operator sequence represent its Hermitian conjugate?
         * If true, the element will correspond to a real symbol (cf. complex if not) in the NPA matrix.
         */
        [[nodiscard]] constexpr bool is_hermitian() const noexcept { return this->hermitian; }

        /**
         * Does the operator sequence represent its Hermitian conjugate up to a minus sign.
         */
        [[nodiscard]] constexpr bool is_antihermitian() const noexcept { return this->antihermitian; }

        /**
         * The real and imaginary offsets of this symbol in the basis (or -1, if no such offset).
         * @return Pair, first corresponding to real, second corresponding to imaginary.
         */
        [[nodiscard]] constexpr std::pair<ptrdiff_t, ptrdiff_t> basis_key() const noexcept {
            return {this->real_index, this->img_index};
        }

        /**
         * Named constructor for entry associated with '0'.
         * @param context The operator context.
         */
        inline static Symbol Zero(const Context& context) {
            auto us = Symbol{OperatorSequence::Zero(context)};
            us.id = 0;
            us.hermitian = true;
            us.antihermitian = true;
            return us;
        }

        /**
         * Named constructor for entry associated with '1'.
         * @param context The operator context.
         */
        inline static Symbol Identity(const Context& context) {
            auto us = Symbol{OperatorSequence::Identity(context)};
            us.id = 1;
            us.hermitian = true;
            us.antihermitian = false;
            us.real_index = 0;
            return us;
        }

        /**
         * Output unique sequence entry, as debug info
         */
        friend std::ostream& operator<<(std::ostream& os, const Symbol& seq);

        friend class SymbolTable;
    };
}