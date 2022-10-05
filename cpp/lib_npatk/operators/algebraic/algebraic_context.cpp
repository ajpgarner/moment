/**
 * algebraic_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_context.h"

namespace NPATK {

    AlgebraicContext::AlgebraicContext(const size_t operator_count)
        : Context{operator_count}, rawSequences{*this}
    {

    }

    AlgebraicContext::AlgebraicContext(const size_t operator_count, std::vector<MonomialSubstitutionRule> rules)
        : Context{operator_count}, rawSequences{*this}, monomialRules{std::move(rules)}
    {

    }

    std::set<symbol_name_t> AlgebraicContext::one_substitution(const RawSequence& input_sequence) const {
        if (input_sequence.size() > this->rawSequences.longest_sequence()) {
            throw errors::bad_substitution(
                    "Cannot perform substitution on strings longer than longest generated string in RawSequenceBook.",
                    input_sequence.operators);
        }

        std::set<symbol_name_t> output;
        for (const auto& rule : this->monomialRules) {
            rule.all_matches(output, this->rawSequences, input_sequence);
        }

        return output;
    }

    bool AlgebraicContext::generate_aliases(size_t level) {
        // Last generated sequence (+1)
        const size_t initial_sequence_count = this->rawSequences.size();

        // Make sure raw sequence book has symbols of correct length
        if (!this->rawSequences.generate(level)) {
            // Early exit if no new strings generated.
            return false;
        }
        const size_t num_sequences = this->rawSequences.size();

        // TODO: Pull up existing transformation rules from tree.

        // Now, apply every transformation rule to every part of every sequence
        for (size_t sequence_index = initial_sequence_count; sequence_index < num_sequences; ++sequence_index) {
            auto out_set = this->one_substitution(this->rawSequences[sequence_index]);
        }

        // TODO: Build and simplify new tree

        return true;
    }

}