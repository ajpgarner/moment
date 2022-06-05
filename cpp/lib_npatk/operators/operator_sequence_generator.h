/**
 * unique_operator_strings.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"
#include "operator_collection.h"


namespace NPATK {
    /**
     * Range over all unique permutations of operators in the supplied context.
     */
    class OperatorSequenceGenerator {
    private:
        const OperatorCollection& context;
        std::vector<OperatorSequence> unique_sequences;
    public:
        /** The maximum length of operator sequence */
        const size_t sequence_length;
    public:
        OperatorSequenceGenerator(const OperatorCollection& operatorContext, size_t sequence_length);

        /**
         * Creates a generator for the piece-wise conjugated OperatorSequences of this generator.
         */
        [[nodiscard]] OperatorSequenceGenerator conjugate() const;


        [[nodiscard]] constexpr auto begin() const noexcept { return unique_sequences.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return unique_sequences.end(); }
        [[nodiscard]] constexpr size_t size() const noexcept { return unique_sequences.size(); }
        [[nodiscard]] constexpr bool empty() const noexcept { return unique_sequences.empty(); }
        [[nodiscard]] constexpr const OperatorSequence& operator[](size_t index) const noexcept {
            assert(index < unique_sequences.size());
            return this->unique_sequences[index];
        };

    private:
        /**
         * Private constructor, assigns supplied seq as unique operator sequences.
         */
        OperatorSequenceGenerator(const OperatorCollection& operatorContext, size_t sequence_length,
                                  std::vector<OperatorSequence>&& seq);


    };
}