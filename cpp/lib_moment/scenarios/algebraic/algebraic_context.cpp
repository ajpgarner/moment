/**
 * algebraic_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_context.h"

#include "symbolic/symbol_set.h"

#include <algorithm>
#include <sstream>

namespace Moment::Algebraic {

    AlgebraicContext::AlgebraicContext(const size_t operator_count, const bool hermitian, const bool commute,
                                       const std::vector<MonomialSubstitutionRule>& initial_rules)
        : Context{operator_count}, self_adjoint{hermitian}, commutative{commute},
          rules{this->hasher, initial_rules, hermitian}
    {
        if (this->commutative) {
            auto extra_rules = RuleBook::commutator_rules(this->hasher, static_cast<oper_name_t>(operator_count));
            this->rules.add_rules(extra_rules);
        }
    }

    AlgebraicContext::~AlgebraicContext() noexcept = default;



    bool AlgebraicContext::attempt_completion(size_t max_attempts, RuleLogger * logger) {
        return this->rules.complete(max_attempts, logger);
    }

    bool AlgebraicContext::additional_simplification(std::vector<oper_name_t>& op_sequence, bool& negated) const {
        if (this->commutative) {
            std::sort(op_sequence.begin(), op_sequence.end());
        }

        HashedSequence inputSeq{std::move(op_sequence), this->hasher};

        auto [reduced, ruleNeg] = this->rules.reduce(inputSeq);

        // Simplify to zero?
        if (reduced.zero()) {
            op_sequence.clear();
            return true;
        }

        // Copy non-zero replacement
        op_sequence.clear();
        op_sequence.reserve(reduced.size());
        for (const auto& op : reduced) {
            op_sequence.emplace_back(op);
        }

        // Negate, if required.
        negated = (negated != ruleNeg);
        return false;
    }

    std::string AlgebraicContext::to_string() const {
        const size_t rule_count = this->rules.size();

        std::stringstream ss;
        ss << "Algebraic context with "
           << this->operator_count << (this->operator_count ? " operators" : " operator")
           << " and " << rule_count << ((rule_count!= 1) ? " rules" : " rule") << ".\n";
        if (this->commutative) {
            ss << "Commutative mode.\n";
        }
        if (this->self_adjoint) {
            ss << "Hermitian mode.\n";
        }
        ss << "Operators: ";
        bool done_one = false;
        for (size_t index = 0; index < this->operator_count; ++index) {
            if (done_one) {
                ss << ", ";
            }
            ss << "X" << index;
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

    std::string AlgebraicContext::resolved_rules() const {
        std::stringstream ss;
        ss << "Rules: \n";
        for (const auto& [id, msr] : this->rules.rules()) {
            ss << "\t" << msr << "\n";
        }
        return ss.str();
    }
}