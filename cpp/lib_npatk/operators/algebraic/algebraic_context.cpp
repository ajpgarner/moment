/**
 * algebraic_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_context.h"

#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

namespace NPATK {

    AlgebraicContext::AlgebraicContext(const size_t operator_count)
        : Context{operator_count}, rawSequences{*this}
    {
        this->buildSet = std::make_unique<SymbolSet>();
        this->buildSet->add_or_merge(Symbol{1});
    }

    AlgebraicContext::AlgebraicContext(const size_t operator_count, std::vector<MonomialSubstitutionRule> rules)
        : Context{operator_count}, rawSequences{*this}, monomialRules{std::move(rules)}
    {
        this->buildSet = std::make_unique<SymbolSet>();
        this->buildSet->add_or_merge(Symbol{1});
    }

    AlgebraicContext::~AlgebraicContext() noexcept = default;

    size_t AlgebraicContext::one_substitution(std::vector<SymbolPair>& output,
                                              const RawSequence& input_sequence) const {
        if (input_sequence.size() > this->rawSequences.longest_sequence()) {
            [[unlikely]]
            throw errors::bad_substitution(
                    "Cannot perform substitution on strings longer than longest generated string in RawSequenceBook.",
                    input_sequence.operators);
        }

        size_t num_pairs = 0;
        for (const auto& rule : this->monomialRules) {
            num_pairs += rule.all_matches(output, this->rawSequences, input_sequence);
        }

        return num_pairs;
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

        // Get existing symbol set, and ensure all new sequences are registered...
        assert(this->buildSet);
        SymbolSet& symbolSet = *this->buildSet;
        for (size_t symbol_index = initial_sequence_count; symbol_index < num_sequences; ++symbol_index) {
            symbolSet.add_or_merge(Symbol{static_cast<symbol_name_t>(symbol_index)});
        }

        // Now, apply every transformation rule to every part of every sequence
        size_t num_matches = 0;
        std::vector<SymbolPair> symbol_pairs;
        for (size_t sequence_index = initial_sequence_count; sequence_index < num_sequences; ++sequence_index) {
            num_matches += this->one_substitution(symbol_pairs, this->rawSequences[sequence_index]);
        }

        // Register discovered pairs...
        for (const auto& pair : symbol_pairs) {
            symbolSet.add_or_merge(pair);
        }

        symbolSet.pack();

        // Do simplification
        SymbolTree theTree{symbolSet};
        theTree.simplify();

        // Recover links for building tree
        this->buildSet = theTree.export_symbol_set();

        // Finally, create map of hashes of sequences to be substituted
        this->hashToReplacementSymbol.clear();
        for (const auto& link : this->buildSet->Links) {
            const auto& target_seq = this->rawSequences[link.first.first];
            const auto& source_seq = this->rawSequences[link.first.second];
            if (link.second != EqualityType::equal) {
                throw std::logic_error{"Currently only equality substitutions are supported..."};
            }
            this->hashToReplacementSymbol.emplace(std::make_pair(source_seq.hash,
                                                                 static_cast<size_t>(target_seq.raw_id)));

        }

        return true;
    }

    bool AlgebraicContext::additional_simplification(std::vector<Operator>& op_sequence) const {
        auto the_hash = this->hash(op_sequence);
        auto ruleIter = this->hashToReplacementSymbol.find(the_hash);

        // No rules, no change.
        if (ruleIter == this->hashToReplacementSymbol.end()) {
            return false;
        }

        // Simplify to zero?
        if (ruleIter->second == 0) {
            op_sequence.clear();
            return true;
        }

        // Copy non-zero replacement
        const auto& replacement = this->rawSequences[ruleIter->second];
        op_sequence.clear();
        op_sequence.reserve(replacement.operators.size());
        for (const auto& op : replacement.operators) {
            op_sequence.emplace_back(op);
        }
        return false;
    }

}