/**
 * operator_sequence_generator.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "scenarios/operator_sequence.h"
#include "scenarios/context.h"

namespace Moment {
    /**
     * Range over all unique permutations of operators in the supplied context.
     */
    class OperatorSequenceGenerator {
    protected:
        /** Context to pull operators from */
        const Context& context;

        /** List of unique sequences */
        std::vector<OperatorSequence> unique_sequences;

    public:
        /** The minimum length of operator sequence */
        const size_t min_sequence_length;

        /** The maximum length of operator sequence */
        const size_t max_sequence_length;


    protected:
        /**
         * OSG template, from list of sequences.
         * @param operatorContext
         * @param min_length Shortest operator sequence
         * @param max_length Longest  operator sequence
         */
        OperatorSequenceGenerator(const Context& operatorContext, std::vector<OperatorSequence>&& unique_sequences,
                                  size_t min_length, size_t max_length)
              : context{operatorContext}, unique_sequences{std::move(unique_sequences)},
                min_sequence_length{min_length}, max_sequence_length{max_length} { }

    public:
        /**
         * Generates all unique permutations of operator sequences, from min_length up to max_length.
         * @param operatorContext
         * @param min_length Shortest operator sequence
         * @param max_length Longest  operator sequence
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t min_length, size_t max_length)
            : OperatorSequenceGenerator{operatorContext,
                                        OperatorSequenceGenerator::build_generic_sequences(operatorContext,
                                                                                           min_length, max_length),
                                            min_length, max_length} {

        }

        /**
         * Move construct OSG
         */
        OperatorSequenceGenerator(OperatorSequenceGenerator&& rhs) = default;


        /**
          * Generates all unique permutations of operator sequences, up to sequence_length.
          * @param operatorContext
          * @param sequence_length
          */
        OperatorSequenceGenerator(const Context& operatorContext, size_t sequence_length)
            : OperatorSequenceGenerator(operatorContext, 0, sequence_length) { }


        /**
         * Create a generator with a list of pre-calculated operator sequences
         * @param operatorContext Reference to context
         * @param min_length The shortest sequence in the pre-computed list
         * @param max_length The longest sequence in the pre-computed list
         * @param preComputedSequences An ordered list of operator sequences the generator will produce
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t min_length, size_t max_length,
                                  std::vector<OperatorSequence>&& preComputedSequences)
                                  : context{operatorContext},
                                    min_sequence_length{min_length}, max_sequence_length{max_length},
                                    unique_sequences{std::move(preComputedSequences)} { }


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
        build_generic_sequences(const Context &context, size_t min_len, size_t max_len);


        [[nodiscard]] constexpr auto begin() const noexcept { return unique_sequences.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return unique_sequences.end(); }
        [[nodiscard]] constexpr size_t size() const noexcept { return unique_sequences.size(); }
        [[nodiscard]] constexpr const OperatorSequence& operator[](size_t index) const noexcept {
            assert(index < unique_sequences.size());
            return this->unique_sequences[index];
        };
    };
}