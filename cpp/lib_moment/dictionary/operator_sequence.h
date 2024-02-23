/**
 * operator_sequence.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "hashed_sequence.h"
#include "hermitian_type.h"

#include <iosfwd>
#include <iterator>
#include <string>
#include <vector>

namespace Moment {

    class Context;
    class ContextualOS;

    /**
     * Represents a sequence of operators, in canonical order with all known simplifications applied.
     * In particular, can be seen as a HashedSequence attached to a Context.
     */
    class OperatorSequence : public HashedSequence {
    public:

        /** Flag to use in constructor, when no simplification or rehashing should be done. */
        struct ConstructRawFlag { };

        /** Flag to use in constructor when no simplification should be done, but hashing is still required. */
        struct ConstructPresortedFlag{ };

    private:
        const Context * context;

        /**
         * Uninitialized constructor, only allowed privately.
         */
         OperatorSequence() = default;

    public:
        /**
         * Constructs empty operator sequence; treated as identity.
         * @param context (Non-owning) point to the Context (if any) for further simplification.
         */
        constexpr explicit OperatorSequence(const Context& context, const bool is_zero = false) noexcept
            : HashedSequence{is_zero}, context{&context} { }

        /**
         * Constructs a sequence of operators, in canonical order, with all known simplifications applied.
         * @param operators A list of operators to include in the sequence
         * @param context Context for further simplification.
         * @param sign_type Whether to interpret the sequence with +1, +i, -1, -i in front of it.
         */
        OperatorSequence(sequence_storage_t operators, const Context& context,
                         SequenceSignType sign_type = SequenceSignType::Positive) noexcept;

        /**
         * Constructs a sequence of operators, with no further simplifications or rehashing.
         * Undefined behaviour if hash is incorrect, or input operators are unsimplified.
         * @param operators A list of operators to include in the sequence
         * @param context Associated context.
         * @param sign_type Whether to interpret the sequence with +1, +i, -1, -i in front of it.
         */
         OperatorSequence(const ConstructRawFlag&, sequence_storage_t operators, uint64_t hash,
                          const Context& context, SequenceSignType sign_type = SequenceSignType::Positive) noexcept
              : HashedSequence{std::move(operators), hash, sign_type}, context{&context} {
             // No simplification, or check-sum of hash!
         }

         /**
         * Constructs a sequence of operators, with no further simplifications, but hashing is required.
         * Undefined behaviour if input operators are unsimplified.
         * @param operators A list of operators to include in the sequence.
         * @param context Associated context.
         * @param sign_type Whether to interpret the sequence with +1, +i, -1, -i in front of it.
         */
         OperatorSequence(const ConstructPresortedFlag&, sequence_storage_t operators,
                          const Context& context, SequenceSignType sign_type = SequenceSignType::Positive) noexcept;

        constexpr OperatorSequence(const OperatorSequence& rhs) = default;

        constexpr OperatorSequence(OperatorSequence&& rhs) noexcept = default;

        OperatorSequence& operator=(const OperatorSequence& rhs) = default;

        OperatorSequence& operator=(OperatorSequence&& rhs) noexcept = default;

        [[nodiscard]] OperatorSequence conjugate() const;

        /**
         * Get operator sequence as a context-formatted string.
         */
        [[nodiscard]] std::string formatted_string() const;

        /**
         * Stream-out operator sequence as a context-formatted string.
         */
        friend std::ostream& operator<<(ContextualOS& os, const OperatorSequence& seq);

        /**
         * Stream-out operator sequence as a context-formatted string.
         */
        friend std::ostream& operator<<(std::ostream& os, const OperatorSequence& seq);


        /**
         * True if supplied context matches
         */
        [[nodiscard]] inline bool is_same_context(const Context& rhs) const noexcept {
            return this->context == &rhs;
        }

        /**
         * Adds a list of operators to the end of the sequence, then simplifies to canonical form.
         * Effectively, a type of multiplication.
         * @tparam iter_t Input iterator type
         * @param begin Input iterator to start of sequence to append.
         * @param end Input iterator to past-the-end of sequence to append.
         */
        template<std::input_iterator iter_t>
        inline OperatorSequence& append(iter_t begin, iter_t end) {
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
        OperatorSequence& operator*= (const OperatorSequence& rhs);

        /**
        * Concatenates two OperatorSequences, putting the output in a new sequence, and simplifying to canonical form.
        * @param lhs The operator sequence to take as the beginning of the new sequence
        * @param rhs The operator sequence to take as the end of the new sequence.
        */
        friend OperatorSequence operator* (const OperatorSequence& lhs, const OperatorSequence& rhs);

        /**
         * Unary minus: copies OperatorSequence, but with sign-type negated.
         */
         friend OperatorSequence operator-(const OperatorSequence& input) {
             if (input.zero()) {
                 return OperatorSequence::Zero(*input.context);
             }
             // Copy construct
             return OperatorSequence{ConstructRawFlag{}, input.operators, input.hash(),
                                     *input.context, negate(input.get_sign())};
         }

         /**
         * Unary minus: negates sign-type of operator sequence.
         */
         friend OperatorSequence operator-(OperatorSequence&& input) noexcept {
             if (input.zero()) {
                 return input;
             }
             input.set_sign(negate(input.get_sign()));
             return input;
         }

        /**
         * Calculates if element is (anti-)Hermitian, by comparing to its conjugate.
         */
        [[nodiscard]] HermitianType hermitian_type() const;

        /**
         * Construct sequence equal to algebraic zero.
         * @param context The scenario to associate with the constructed sequence.
         * @return Zero.
         */
        [[nodiscard]] static OperatorSequence Zero(const Context& context) {
            OperatorSequence output{context};
            output.the_hash = 0;
            return output;
        }

        /**
         * Construct sequence equal to algebraic identity.
         * @param context The scenario to associate with the constructed sequence.
         * @return One.
         */
        [[nodiscard]] static OperatorSequence Identity(const Context& context) {
            return OperatorSequence{context};
        }


        /**
         * Construct sequence equal to algebraic identity up to a sign.
         * @param context The scenario to associate with the constructed sequence.
         * @param sign_type The sign of the sequence
         * @return +1, +i, -1 or -i depending on sign_type.
         */
        [[nodiscard]] static OperatorSequence Identity(const Context& context, SequenceSignType sign_type) {
            OperatorSequence id{context};
            id.set_sign(sign_type);
            return id;
        }

        /**
         * Create a block of (mostly) uninitialized operator sequences for overwrite.
         * @param elements The number of sequences to make in the block.
         */
        static std::vector<OperatorSequence> create_uninitialized_vector(const size_t elements) {
            return std::vector<OperatorSequence>(elements, OperatorSequence{});
        }


    private:
        /**
         * Perform simplifications on the raw operator sequence, calling context if supplied.
         */
        void to_canonical_form() noexcept;



        friend class Context;
    };


}