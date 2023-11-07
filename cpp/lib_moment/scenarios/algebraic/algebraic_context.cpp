/**
 * algebraic_context.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_context.h"

#include "name_table.h"

#include "dictionary/operator_sequence.h"

#include <algorithm>
#include <sstream>

namespace Moment::Algebraic {



    AlgebraicContext::AlgebraicContext(const AlgebraicPrecontext& input_apc,
                                       std::unique_ptr<NameTable> names,
                                       const bool commute, const bool normal,
                                       const std::vector<OperatorRule>& initial_rules)
        : Context{static_cast<size_t>(input_apc.num_operators)},
          precontext{input_apc},
          self_adjoint{input_apc.self_adjoint()},
          commutative{commute}, rules{precontext, initial_rules}, op_names{std::move(names)} {

        // Make rules
        if (this->commutative) {
            auto extra_rules = OperatorRulebook::commutator_rules(this->precontext);
            this->rules.add_rules(extra_rules);
        }
        if (!this->self_adjoint && normal) {
            auto extra_rules = OperatorRulebook::normal_rules(this->precontext);
            this->rules.add_rules(extra_rules);
        }

        // If no rules, then automatically complete
        if (this->rules.size() == 0) {
            this->rules_completed.emplace(true);
        }
    }


    AlgebraicContext::AlgebraicContext(const AlgebraicPrecontext& apc, bool commute, bool normal,
                                       const std::vector<OperatorRule>& rules)
            : AlgebraicContext{apc, std::make_unique<NameTable>(apc), commute, normal, rules} {
        // delegated.
    }


    AlgebraicContext::AlgebraicContext(oper_name_t num_ops)
            : AlgebraicContext{AlgebraicPrecontext(num_ops), false, true, {}} {
        // delegated.

    }


    AlgebraicContext::~AlgebraicContext() noexcept = default;


    std::optional<OperatorSequence> AlgebraicContext::get_if_canonical(const sequence_storage_t &sequence) const {
        if (this->rules.can_reduce(sequence)) {
            return std::nullopt;
        }
        return std::make_optional<OperatorSequence>(OperatorSequence::ConstructRawFlag{},
                                                    sequence, this->hash(sequence), *this);
    }


    bool AlgebraicContext::attempt_completion(size_t max_attempts, RuleLogger * logger) {
        this->rules_completed.emplace(this->rules.complete(max_attempts, logger));
        return this->rules_completed.value();
    }

    bool AlgebraicContext::is_complete() {
        if (!this->rules_completed.has_value()) {
            this->rules_completed.emplace(this->rules.is_complete());
        }
        return this->rules_completed.value();
    }

    bool AlgebraicContext::is_complete() const {
        if (!this->rules_completed.has_value()) {
            throw std::runtime_error{"It has not yet been checked whether the rules are complete."};
        }
        return this->rules_completed.value();
    }

    bool AlgebraicContext::additional_simplification(sequence_storage_t& op_sequence, SequenceSignType& sign_type) const {
        if (this->commutative) {
            std::sort(op_sequence.begin(), op_sequence.end());
        }

        const auto result = this->rules.reduce_in_place(op_sequence, sign_type);
        switch (result) {
            case OperatorRulebook::RawReductionResult::SetToZero:
                op_sequence.clear();
                return true;
            default:
                break;
        }

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
            ss << "MonomialRules: \n";
            for (const auto& [id, msr] : this->rules.rules()) {
                ss << "\t" << msr << "\n";
            }
        }

        return ss.str();
    }

    std::string AlgebraicContext::resolved_rules() const {
        std::stringstream ss;
        ss << "MonomialRules: \n";
        for (const auto& [id, msr] : this->rules.rules()) {
            ss << "\t" << msr << "\n";
        }
        return ss.str();
    }

    OperatorSequence AlgebraicContext::conjugate(const OperatorSequence &seq) const {
        // If context only has self-adjoint operators, use default function
        if (this->self_adjoint) {
            return Context::conjugate(seq);
        }

        // 0* = 0
        if (seq.zero()) {
            return OperatorSequence::Zero(*this);
        }

        return OperatorSequence{this->precontext.conjugate(seq.raw()), *this};
    }


    void AlgebraicContext::format_sequence(ContextualOS &os, const OperatorSequence &seq) const {
        if (seq.zero()) {
            os.os << "0";
            return;
        }

        if (seq.empty()) {
            os.os << "1";
            return;
        }

        if (seq.negated()) {
            os.os << "-";
        }

        if (os.format_info.show_braces) {
            os << "<";
        }

        if (this->op_names->all_single()) {
            for (const auto &oper: seq) {
                os.os << (*this->op_names)[oper];
            }
        } else {
            bool done_once = false;
            for (const auto &oper: seq) {
                if (done_once) {
                    os.os << ";";
                } else {
                    done_once = true;
                }
                this->names().format_stream(os.os, oper);
            }
        }

        if (os.format_info.show_braces) {
            os << ">";
        }

    }


    void AlgebraicContext::format_raw_sequence(ContextualOS &os, const sequence_storage_t &seq) const {
        if (seq.empty()) {
            os.os << "1";
            return;
        }

        if (os.format_info.show_braces) {
            os << "<";
        }

        if (this->op_names->all_single()) {
            for (const auto& oper: seq) {
                os.os << (*this->op_names)[oper];
            }
        } else {
            bool done_once = false;
            for (const auto &oper: seq) {
                if (done_once) {
                    os.os << ";";
                } else {
                    done_once = true;
                }
                this->names().format_stream(os.os, oper);
            }
        }

        if (os.format_info.show_braces) {
            os << ">";
        }
    }


    std::unique_ptr<AlgebraicContext>
    AlgebraicContext::FromNameList(std::initializer_list<std::string> names) {
        return std::make_unique<AlgebraicContext>(
            AlgebraicPrecontext(static_cast<oper_name_t>(names.size()), AlgebraicPrecontext::ConjugateMode::SelfAdjoint),
            std::make_unique<NameTable>(names), false, true, std::vector<OperatorRule>{}
        );
    }


}