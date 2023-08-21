/**
 * ostream_rule_logger.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "ostream_rule_logger.h"
#include "name_table.h"

#include <iostream>


namespace Moment::Algebraic {

    namespace {
        /** Utility class for std::ostream<< with name-table information. */
        class RuleFormatter {
        public:
            const NameTable* names = nullptr;
            const OperatorRule &rule;

            explicit constexpr RuleFormatter(const NameTable* names, const OperatorRule &rule)
                : names{names}, rule{rule} { }

            friend std::ostream& operator<<(std::ostream& os, const RuleFormatter& rf) {
                if (rf.names) {
                    if (rf.rule.LHS().empty()) {
                        if (rf.rule.LHS().hash() != 0) {
                            os << "I";
                        } else {
                            os << "0";
                        }
                    } else {
                        rf.names->format_stream(os, rf.rule.LHS().begin(), rf.rule.LHS().end());
                    }
                    os << " -> ";
                    if (rf.rule.negated()) {
                        os << "-";
                    }
                    if (rf.rule.RHS().empty()) {
                        if (rf.rule.RHS().hash() != 0) {
                            os << "I";
                        } else {
                            os << "0";
                        }
                    } else {
                        rf.names->format_stream(os, rf.rule.RHS().begin(), rf.rule.RHS().end());
                    }
                } else {
                    os << rf.rule;
                }
                return os;
            }
        };
    }

    void OStreamRuleLogger::rule_reduced(const OperatorRule& old_rule,
                                         const OperatorRule& new_rule) {
        os << "Reduce:\t" << RuleFormatter(names, old_rule)
           << "\n  |-\t" << RuleFormatter(names, new_rule) << "\n";
    }

    void OStreamRuleLogger::rule_removed(const OperatorRule& ex_rule) {
        os << "Remove:\t" << RuleFormatter(names, ex_rule) << "\n";
    }

    void OStreamRuleLogger::rule_introduced(const OperatorRule& parent_rule_a,
                                            const OperatorRule& parent_rule_b,
                                            const OperatorRule& new_rule) {
        os << "Combine:\t" << RuleFormatter(names, parent_rule_a)
           << "\tand " << RuleFormatter(names, parent_rule_b) << ":"
           << "\n  |-\t" << RuleFormatter(names, new_rule) << "\n";
    }

    void OStreamRuleLogger::rule_introduced(const OperatorRule& new_rule) {
        os << "Directly added:\t" << RuleFormatter(names, new_rule) << "\n";
    }

    void OStreamRuleLogger::rule_introduced_conjugate(const OperatorRule& parent_rule,
                                                      const OperatorRule& new_rule) {
        os << "Conjugate:\t" << RuleFormatter(names, parent_rule) << ":"
           << "\n  |-\t" << RuleFormatter(names, new_rule) << "\n";
    }


    void OStreamRuleLogger::success(const OperatorRulebook& rb, size_t attempts) {
        os << "The rule set was successfully completed after " << attempts
           << " new rule" << ((attempts != 1) ? "s" : "") << ".\n";
    }

    void OStreamRuleLogger::failure(const OperatorRulebook& rb, size_t attempts) {
        os << "The rule set is still incomplete, after " << attempts
           << " new rule" << ((attempts != 1) ? "s" : "") << ".\n";
    }

}