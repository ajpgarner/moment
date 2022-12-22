/**
 * ostream_rule_logger.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "ostream_rule_logger.h"

#include <iostream>

namespace Moment::Algebraic {

    void OStreamRuleLogger::rule_reduced(const MonomialSubstitutionRule& old_rule,
                                         const MonomialSubstitutionRule& new_rule) {
        os << "Reduce:\t" << old_rule << "\n  |-\t" << new_rule << "\n";
    }

    void OStreamRuleLogger::rule_removed(const MonomialSubstitutionRule& ex_rule) {
        os << "Remove:\t" << ex_rule << "\n";
    }

    void OStreamRuleLogger::rule_introduced(const MonomialSubstitutionRule& parent_rule_a,
                                            const MonomialSubstitutionRule& parent_rule_b,
                                            const MonomialSubstitutionRule& new_rule) {
        os << "Combine:\t" << parent_rule_a << "\tand " << parent_rule_b << ":"
           << "\n  |-\t" << new_rule << "\n";
    }

    void OStreamRuleLogger::rule_introduced(const MonomialSubstitutionRule& new_rule) {
        os << "Directly added:\t" << new_rule << "\n";
    }

    void OStreamRuleLogger::rule_introduced_conjugate(const MonomialSubstitutionRule& parent_rule,
                                                      const MonomialSubstitutionRule& new_rule) {
        os << "Conjugate:\t" << parent_rule << ":"
           << "\n  |-\t" << new_rule << "\n";
    }


    void OStreamRuleLogger::success(const RuleBook& rb, size_t attempts) {
        os << "The rule set was successfully completed after " << attempts
           << " new rule" << ((attempts != 1) ? "s" : "") << ".\n";
    }

    void OStreamRuleLogger::failure(const RuleBook& rb, size_t attempts) {
        os << "The rule set is still incomplete, after " << attempts
           << " new rule" << ((attempts != 1) ? "s" : "") << ".\n";
    }

}