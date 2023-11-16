/**
 * operator_sequence_generator.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_sequence.h"
#include "scenarios/context.h"

namespace Moment {
    /**
     * Range over all unique permutations of operators in the supplied context.
     */
    class OperatorSequenceGenerator {
    public:
        /** Context to pull operators from */
        const Context& context;

    protected:
        /** List of unique sequences */
        std::vector<OperatorSequence> unique_sequences;

    public:
        /** The maximum length of operator sequence */
        const size_t max_sequence_length;


    protected:


        /**
         * Create a generator with a list of pre-calculated operator sequences
         * @param operatorContext Reference to context
         * @param max_length The longest sequence in the pre-computed list
         * @param preComputedSequences An ordered list of operator sequences the generator will produce
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t max_length,
                                  std::vector<OperatorSequence>&& preComputedSequences)
                : context{operatorContext}, max_sequence_length{max_length},
                  unique_sequences{std::move(preComputedSequences)} { }


    public:
        /**
         * Generates all unique permutations of operator sequences, up to max_length.
         * @param operatorContext
         * @param max_length Longest  operator sequence
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t max_length)
            : OperatorSequenceGenerator{operatorContext, max_length,
                OperatorSequenceGenerator::build_generic_sequences(operatorContext, max_length)} { }

        /**
         * Move construct OSG
         */
        OperatorSequenceGenerator(OperatorSequenceGenerator&& rhs) = default;



        virtual ~OperatorSequenceGenerator() noexcept = default;

        /**
         * Creates a generator for the piece-wise conjugated OperatorSequences of this generator.
         */
        [[nodiscard]] OperatorSequenceGenerator conjugate() const;

        /**
         * Create all generic sequences
         * @return
         */
        static std::vector<OperatorSequence>
        build_generic_sequences(const Context &context, size_t max_len);


        [[nodiscard]] constexpr auto begin() const noexcept { return unique_sequences.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return unique_sequences.end(); }
        [[nodiscard]] constexpr size_t size() const noexcept { return unique_sequences.size(); }
        [[nodiscard]] constexpr const OperatorSequence& operator[](size_t index) const noexcept {
            assert(index < unique_sequences.size());
            return this->unique_sequences[index];
        };
    };
}