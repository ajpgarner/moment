/**
 * ostream_rule_logger.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_rulebook.h"

#include <iosfwd>

namespace Moment::Algebraic {

    class NameTable;

    class OStreamRuleLogger : public RuleLogger {
    private:
        std::ostream& os;
        const NameTable * names;

    public:
        explicit OStreamRuleLogger(std::ostream& stream, const NameTable* names = nullptr) : os{stream}, names{names} {

        }

        void rule_reduced(const OperatorRule& old_rule,
                          const OperatorRule& new_rule) override;

        void rule_removed(const OperatorRule& ex_rule) override;

        void rule_introduced(const OperatorRule& new_rule) override;

        void rule_introduced(
                const OperatorRule& parent_rule_a,
                const OperatorRule& parent_rule_b,
                const OperatorRule& new_rule) override;

        void rule_introduced_conjugate(
                const OperatorRule& parent_rule,
                const OperatorRule& new_rule) override;

        void success(const OperatorRulebook& rb, size_t attempts) override;

        void failure(const OperatorRulebook& rb, size_t attempts) override;

    };
}