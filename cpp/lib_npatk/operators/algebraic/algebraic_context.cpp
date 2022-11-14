/**
 * algebraic_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_context.h"

#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include "operators/operator_sequence.h"

#include <algorithm>
#include <sstream>

namespace NPATK {

    AlgebraicContext::AlgebraicContext(const size_t operator_count, const bool hermitian, const bool commute,
                                       const std::vector<MonomialSubstitutionRule>& initial_rules)
        : Context{operator_count}, self_adjoint{hermitian}, commutative{commute},
          rawSequences{*this}, rules{this->hasher, initial_rules, hermitian}
    {
        if (this->commutative) {
            auto extra_rules = RuleBook::commutator_rules(this->hasher, static_cast<oper_name_t>(operator_count));
            this->rules.add_rules(extra_rules);
        }

        this->generate_aliases(0);
    }

    AlgebraicContext::~AlgebraicContext() noexcept = default;


    std::string AlgebraicContext::to_string() const {
        const size_t op_count = this->operators.size();
        const size_t rule_count = this->rules.size();

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
            for (const auto& [id, msr] : this->rules.rules()) {
                ss << "\t" << msr << "\n";
            }
        }

        return ss.str();
    }

    bool AlgebraicContext::attempt_completion(size_t max_attempts, RuleLogger * logger) {
        return this->rules.complete(max_attempts, logger);
    }


    size_t AlgebraicContext::one_substitution(std::vector<SymbolPair>& output,
                                              const RawSequence& input_sequence) const {
        if (input_sequence.size() > this->rawSequences.longest_sequence()) {
            [[unlikely]]
            throw errors::bad_substitution{
                    "Cannot perform substitution on strings longer than longest generated string in RawSequenceBook."};
        }

        size_t num_pairs = 0;
        for (const auto& [hash, rule] : this->rules.rules()) {
            num_pairs += rule.all_matches(output, this->rawSequences, input_sequence);
        }

        return num_pairs;
    }

    bool AlgebraicContext::generate_aliases(size_t level) {
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

        // Synchronize symbols with deduced zeros
        this->rawSequences.synchronizeNullity(*this->buildSet);

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
            const auto& target_seq = rawSequences[link.first.first];
            const symbol_name_t target_id = is_conjugated(link.second) ? target_seq.conjugate_id : target_seq.raw_id;
            bool negated = is_negated(link.second);

            // Don't insert reflexive rules...
            if (link.first.second != target_id) {
                this->hashToReplacementSymbol.emplace(std::make_pair(source_seq.hash(),
                                                                     std::make_pair(static_cast<uint64_t>(target_id),
                                                                                    negated)));
            }
        }



    }

    bool AlgebraicContext::additional_simplification(std::vector<oper_name_t>& op_sequence, bool& negated) const {
        auto the_hash = this->hash(op_sequence);
        auto ruleIter = this->hashToReplacementSymbol.find(the_hash);

        // No rules, no change.
        if (ruleIter == this->hashToReplacementSymbol.end()) {
            return false;
        }

        // Simplify to zero?
        if (ruleIter->second.first == 0) {
            op_sequence.clear();
            return true;
        }

        // Copy non-zero replacement
        const auto& replacement = this->rawSequences[ruleIter->second.first];
        op_sequence.clear();
        op_sequence.reserve(replacement.size());
        for (const auto& op : replacement) {
            op_sequence.emplace_back(op);
        }

        // Negate, if required.
        negated = (negated != ruleIter->second.second);

        return false;
    }

    std::pair<bool, bool> AlgebraicContext::is_sequence_null(const OperatorSequence& seq) const noexcept {

        // Can we find this sequence?
        const auto* rawSeqPtr = this->rawSequences.where(seq.hash());
        if (nullptr == rawSeqPtr) {
            return {false, false};
        }

        // Get information from associated symbol
        const symbol_name_t symbol_id = rawSeqPtr->raw_id;
        const auto& symbol = this->rawSequences.Symbols()[symbol_id];
        return {symbol.real_is_zero, symbol.im_is_zero};
    }



    std::string AlgebraicContext::resolved_rules() const {
        std::stringstream ss;
        for (const auto [lhs_hash, rhs_symbol] : this->hashToReplacementSymbol) {
            auto * lhs_raw_ptr = this->rawSequences.where(lhs_hash);
            assert(lhs_raw_ptr != nullptr);
            const auto& lhs_raw = *lhs_raw_ptr;
            const auto& rhs_raw = this->rawSequences[rhs_symbol.first];

            ss << lhs_raw.raw_id << " [";
            for (auto o : lhs_raw) {
                ss << "X" << o;
            }
            ss << "] -> ";

            ss << (rhs_symbol.second ? "-" : "");
            ss << rhs_raw.raw_id << " [";
            ss << (rhs_symbol.second ? "-" : "");
            for (auto o : rhs_raw) {
                ss << "X" << o;
            }
            ss << "]\n";
        }
        return ss.str();
    }


}