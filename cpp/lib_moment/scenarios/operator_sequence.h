/**
 * operator_sequence.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "hashed_sequence.h"

#include <iosfwd>
#include <iterator>
#include <string>
#include <vector>

namespace Moment {

    class Context;

    /**
     * Represents a sequence of Hermitian operators, in canonical order with all known simplifications applied.
     * In particular, can be seen as a HashedSequence attached to a Context.
     */
    class OperatorSequence : public HashedSequence {
    public:
        struct ConstructRawFlag { };

    private:
        const Context& context;

        bool is_negated = false;

    public:
        /**
         * Constructs empty operator sequence; treated as identity.
         * @param context (Non-owning) point to the Context (if any) for further simplification.
         */
        constexpr explicit OperatorSequence(const Context& context)
            : HashedSequence{false}, context{context} { }

        /**
         * Constructs a sequence of operators, in canonical order, with all known simplifications applied.
         * @param operators A list of operators to include in the sequence
         * @param context Context for further simplification.
         * @param negated True if sequence should be interpreted with a minus sign in front of it.
         */
        OperatorSequence(sequence_storage_t operators, const Context& context, bool negated = false) noexcept;

        /**
         * Constructs a sequence of operators, with no further simplifications added
         * @param rhs
         */
         OperatorSequence(const ConstructRawFlag&, sequence_storage_t operators, uint64_t hash,
                          const Context& context,
                          bool negated = false) noexcept
              : HashedSequence{std::move(operators), hash}, context{context}, is_negated{negated} {
             // No simplification, or check-sum of hash!
         }

        constexpr OperatorSequence(const OperatorSequence& rhs) = default;

        constexpr OperatorSequence(OperatorSequence&& rhs) noexcept = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        /**
         * True, if sequence should be interpreted with a negative sign.
         */
        [[nodiscard]] constexpr bool negated() const noexcept { return this->is_negated; }

        /**
         * Get operator sequence as a context-formatted string.
         */
        [[nodiscard]] std::string formatted_string() const;

        /**
         * Stream-out operator sequence as a context-formatted string.
         */
        friend std::ostream& operator<<(std::ostream& os, const OperatorSequence& seq);

        /**
         * Adds a list of operators to the end of the sequence, then simplifies to canonical form.
         * @tparam iter_t Input iterator type
         * @param begin Input iterator to start of sequence to append.
         * @param end Input iterator to past-the-end of sequence to append.
         */
        template<std::input_iterator iter_t>
        inline OperatorSequence& append(iter_t begin, iter_t end) {
            this->operators.reserve(this->operators.size() + std::distance(begin, end));
            this->operators.insert(this->operators.end(), begin, end);
            this->to_canonical_form();
            return *this;
        }

        /**
         * Adds a list of operators to the end of the sequence, then simplifies to canonical form.
         * @param List of operators to append.
         */
        inline OperatorSequence& append(std::initializer_list<oper_name_t> opList) {
            return this->append(opList.begin(), opList.end());
        }

        /**
         * Concatenate an OperatorSequence to the end of this sequence, then simplifies to canonical form.
         * @param rhs The operator sequence to append to this sequence.
         */
        OperatorSequence& operator *= (const OperatorSequence& rhs) {
            return this->append(rhs.operators.begin(), rhs.operators.end());
        }

        /**
         * True if supplied context matches
         */
        inline bool is_same_context(const Context& rhs) const noexcept {
            return &this->context == &rhs;
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

        /**
         * Construct sequence equal to algebraic zero.
         * @param context The scenario to associate with the constructed sequence.
         * @return Zero.
         */
        static OperatorSequence Zero(const Context& context) {
            OperatorSequence output{context};
            output.the_hash = 0;
            output.is_zero = true;
            return output;
        }

        /**
         * Construct sequence equal to algebraic identity.
         * @param context The scenario to associate with the constructed sequence.
         * @return One.
         */
        static OperatorSequence Identity(const Context& context) {
            return OperatorSequence{context};
        }

        /**
         * Compare two sequences for equality or negative-equality.
         * @param lhs First sequence to compare.
         * @param rhs Second sequence to compare.
         * @return +1 if sequences are identical, 0 if they are completely different, -1 if lhs = -rhs.
         */
        [[nodiscard]] static int compare_same_negation(const OperatorSequence& lhs, const OperatorSequence& rhs);



    private:
        /**
         * Perform simplifications on the raw operator sequence, calling context if supplied.
         */
        void to_canonical_form() noexcept;



        friend class Context;
    };


}