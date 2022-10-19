/**
 * algebraic_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_context.h"

#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include <algorithm>
#include <sstream>

namespace NPATK {

    AlgebraicContext::AlgebraicContext(const size_t operator_count, const bool hermitian)
        : Context{operator_count}, self_adjoint{hermitian}, rawSequences{*this}
    {
        this->generate_aliases(0);
    }

    AlgebraicContext::AlgebraicContext(const size_t operator_count, const bool hermitian,
                                       std::vector<MonomialSubstitutionRule> rules)
        : Context{operator_count}, self_adjoint{hermitian}, rawSequences{*this}, monomialRules{std::move(rules)}
    {
        const auto max_length_elem = std::max_element(monomialRules.cbegin(), monomialRules.cend(),
        [](const auto& ruleL, const auto& ruleR) {
            return ruleL.LHS().size() < ruleR.LHS().size();
        });
        const size_t max_length = max_length_elem->LHS().size();
        this->generate_aliases(2*max_length);
    }

    AlgebraicContext::~AlgebraicContext() noexcept = default;


    std::string AlgebraicContext::to_string() const {
        const size_t op_count = this->operators.size();
        const size_t rule_count = this->monomialRules.size();

        std::stringstream ss;
        ss << "Algebraic context with "
           << op_count << (op_count ? " operators" : " operator")
           << " and " << rule_count << ((rule_count!= 1) ? " rules" : " rule") << ".\n";
        ss << "Operators: ";
        bool done_one = false;
        for (size_t index = 0; index < op_count; ++index) {
            if (done_one) {
                ss << ", ";
            }
            ss << "X" << this->operators[index];
            done_one = true;
        }
        ss << "\n";
        if (rule_count > 0) {
            ss << "Rules: \n";
            for (const auto& msr : this->monomialRules) {
                ss << "\t" << msr << "\n";
            }
        }

        return ss.str();
    }


    size_t AlgebraicContext::one_substitution(std::vector<SymbolPair>& output,
                                              const RawSequence& input_sequence) const {
        if (input_sequence.size() > this->rawSequences.longest_sequence()) {
            [[unlikely]]
            throw errors::bad_substitution{
                    "Cannot perform substitution on strings longer than longest generated string in RawSequenceBook."};
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

        // Get symbol set, with symbols and complex-conjugacy information
        this->buildSet = this->rawSequences.symbol_set();
        auto& symbolSet = *this->buildSet;

        // Now, apply every transformation rule to every part of every sequence
        size_t num_matches = 0;
        std::vector<SymbolPair> symbol_pairs;
        for (size_t sequence_index = 0; sequence_index < num_sequences; ++sequence_index) {
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
        build_hash_table();

        return true;
    }

    void AlgebraicContext::build_hash_table() {
        this->hashToReplacementSymbol.clear();
        for (const auto& link : this->buildSet->Links) {
            if (link.first.first == link.first.second) {
                throw std::logic_error{"Self-references should have been resolved in tree simplification!"};
            }
            const auto& source_seq = rawSequences[link.first.second];
            symbol_name_t target_id = -1;
            if (link.second == EqualityType::equal) {
                const auto& target_seq = rawSequences[link.first.first];
                target_id = target_seq.raw_id;
            } else if (link.second == EqualityType::conjugated) {
                const auto& target_seq = rawSequences[link.first.first];
                target_id = target_seq.conjugate_id;
            } else {
                throw std::logic_error{"Currently only equality and conjugation substitutions are supported..."};
            }

            // Don't insert reflexive rules...
            if (link.first.second != target_id) {
                this->hashToReplacementSymbol.emplace(std::make_pair(source_seq.hash,
                                                                     static_cast<size_t>(target_id)));
            }
        }
    }

    bool AlgebraicContext::additional_simplification(std::vector<oper_name_t>& op_sequence) const {
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
        op_sequence.reserve(replacement.size());
        for (const auto& op : replacement) {
            op_sequence.emplace_back(op);
        }
        return false;
    }

    std::string AlgebraicContext::resolved_rules() const {
        std::stringstream ss;
        for (const auto [lhs_hash, rhs_symbol] : this->hashToReplacementSymbol) {
            auto * lhs_raw_ptr = this->rawSequences.where(lhs_hash);
            assert(lhs_raw_ptr != nullptr);
            const auto& lhs_raw = *lhs_raw_ptr;
            const auto& rhs_raw = this->rawSequences[rhs_symbol];

            ss << lhs_raw.raw_id << " [";
            for (auto o : lhs_raw) {
                ss << "X" << o;
            }
            ss << "] -> ";

            ss << rhs_raw.raw_id << " [";
            for (auto o : rhs_raw) {
                ss << "X" << o;
            }
            ss << "]\n";
        }

        return ss.str();
    }


}