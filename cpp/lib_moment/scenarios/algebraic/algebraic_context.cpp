/**
 * algebraic_context.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_context.h"

#include "name_table.h"

#include "scenarios/operator_sequence.h"

#include <algorithm>
#include <sstream>

namespace Moment::Algebraic {



    AlgebraicContext::AlgebraicContext(const AlgebraicPrecontext& input_apc,
                                       std::unique_ptr<NameTable> names,
                                       const bool commute, const bool normal,
                                       const std::vector<MonomialSubstitutionRule>& initial_rules)
        : Context{static_cast<size_t>(input_apc.num_operators)},
          precontext{input_apc},
          self_adjoint{input_apc.self_adjoint()},
          commutative{commute}, rules{precontext, initial_rules}, op_names{std::move(names)} {

        // Make rules
        if (this->commutative) {
            auto extra_rules = RuleBook::commutator_rules(this->precontext);
            this->rules.add_rules(extra_rules);
        }
        if (!this->self_adjoint && normal) {
            auto extra_rules = RuleBook::normal_rules(this->precontext);
            this->rules.add_rules(extra_rules);
        }
    }


    AlgebraicContext::AlgebraicContext(const AlgebraicPrecontext& apc, bool commute, bool normal,
                                       const std::vector<MonomialSubstitutionRule>& rules)
            : AlgebraicContext{apc, std::make_unique<NameTable>(apc), commute, normal, rules} {
        // delegated.
    }


    AlgebraicContext::AlgebraicContext(oper_name_t num_ops)
            : AlgebraicContext{AlgebraicPrecontext(num_ops), false, true, {}} {
        // delegated.
    }


    AlgebraicContext::~AlgebraicContext() noexcept = default;


    bool AlgebraicContext::attempt_completion(size_t max_attempts, RuleLogger * logger) {
        return this->rules.complete(max_attempts, logger);
    }

    bool AlgebraicContext::additional_simplification(sequence_storage_t &op_sequence, bool& negated) const {
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

            ss << (*this->op_names)[index];
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

    OperatorSequence AlgebraicContext::conjugate(const OperatorSequence &seq) const {
        // If self-adjoint, use default function
        if (this->self_adjoint) {
            return Context::conjugate(seq);
        }

        // 0* = 0
        if (seq.zero()) {
            return OperatorSequence::Zero(*this);
        }

        return OperatorSequence(this->precontext.conjugate(seq.raw()), *this);
    }

    std::string AlgebraicContext::format_sequence(const OperatorSequence &seq) const {
        if (seq.zero()) {
            return "0";
        }
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;

        if (seq.negated()) {
            ss << "-";
        }

        if (this->op_names->all_single()) {
            for (const auto &oper: seq) {
                ss << (*this->op_names)[oper];
            }
        } else {
            bool done_once = false;
            for (const auto &oper: seq) {
                if (done_once) {
                    ss << ";";
                } else {
                    done_once = true;
                }
                this->names().format_stream(ss, oper);
            }
        }
        return ss.str();
    }


    std::string AlgebraicContext::format_raw_sequence(const sequence_storage_t &seq) const {
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;
        if (this->op_names->all_single()) {
            for (const auto& oper: seq) {
                ss << (*this->op_names)[oper];
            }
        } else {
            bool done_once = false;
            for (const auto &oper: seq) {
                if (done_once) {
                    ss << ";";
                } else {
                    done_once = true;
                }
                this->names().format_stream(ss, oper);
            }
        }
        return ss.str();
    }


    std::unique_ptr<AlgebraicContext>
    AlgebraicContext::FromNameList(std::initializer_list<std::string> names) {
        return std::make_unique<AlgebraicContext>(
            AlgebraicPrecontext(static_cast<oper_name_t>(names.size()), AlgebraicPrecontext::ConjugateMode::SelfAdjoint),
            std::make_unique<NameTable>(names), false, true, std::vector<MonomialSubstitutionRule>{}
        );
    }


}