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

    class Context;

    /**
     * Represents a sequence of Hermitian operators, in canonical order with all known simplifications applied.
     */
    class OperatorSequence {
    private:
        std::vector<Operator> constituents{};
        const Context * context = nullptr;
        bool is_zero = false;

    public:

        /**
         * Constructs empty operator sequence; treated as identity.
         * @param context (Non-owning) point to the Context (if any) for further simplification.
         */
        constexpr explicit OperatorSequence(const Context * context = nullptr) : context{context} {

        }

        OperatorSequence(std::initializer_list<Operator> operators)
            : constituents(operators) {
            this->to_canonical_form();
        }

        /**
         * Constructs a sequence of Hermitian operators, in canonical order, with all known simplifications applied.
         * @param operators A list of operators to include in the sequence
         * @param context (Non-owning) pointer to the Context (if any) for further simplification.
         */
        explicit OperatorSequence(std::vector<Operator>&& operators, const Context * context = nullptr)
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
         * Removes context from OperatorSequence.
         */
         constexpr void detach() noexcept { this->context = nullptr; }

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


        /**
         * Adds a list of operators to the end of the sequence, then simplifies to canonical form.
         * @tparam iter_t Input iterator type
         * @param begin Input iterator to start of sequence to append.
         * @param end Input iterator to past-the-end of sequence to append.
         */
        template<std::input_iterator iter_t>
        inline OperatorSequence& append(iter_t begin, iter_t end) {
            this->constituents.reserve(this->constituents.size() + std::distance(begin, end));
            this->constituents.insert(this->constituents.end(), begin, end);
            this->to_canonical_form();
            return *this;
        }

        /**
         * Adds a list of operators to the end of the sequence, then simplifies to canonical form.
         * @param List of operators to append.
         */
        inline OperatorSequence& append(std::initializer_list<Operator> opList) {
            return this->append(opList.begin(), opList.end());
        }

        /**
         * Concatenate an OperatorSequence to the end of this sequence, then simplifies to canonical form.
         * @param rhs The operator sequence to append to this sequence.
         */
        OperatorSequence& operator *= (const OperatorSequence& rhs) {
            return this->append(rhs.constituents.begin(), rhs.constituents.end());
        }


        /**
        * Concatenates two OperatorSequences, putting the output in a new sequence, and simplifying to canonical form.
        * @param lhs The operator sequence to take as the beginning of the new sequence
        * @param rhs The operator sequence to take as the end of the new sequence.
        */
        inline friend OperatorSequence operator * (const OperatorSequence& lhs, const OperatorSequence& rhs) {
            OperatorSequence output{lhs};
            output *= rhs;
            return output;
        }

        /**
        * Concatenates two OperatorSequences, putting the output in a new sequence, and simplifying to canonical form.
        * This overload avoids copying the LHS sequence.
        * @param lhs The operator sequence to take as the beginning of the new sequence
        * @param rhs The operator sequence to take as the end of the new sequence.
        */
        inline friend OperatorSequence operator * (OperatorSequence&& lhs, const OperatorSequence& rhs) {
            lhs *= rhs;
            return lhs;
        }

        constexpr static OperatorSequence Zero(const Context * context = nullptr) {
            OperatorSequence output{context};
            output.is_zero = true;
            return output;
        }

        constexpr static OperatorSequence Identity(const Context * context = nullptr) {
            return OperatorSequence{context};
        }

    private:
        /**
         * Perform simplifications on the raw operator sequence, calling context if supplied.
         */
        void to_canonical_form() noexcept;
    };


}