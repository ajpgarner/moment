/**
 * ostream_rule_logger.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "rule_book.h"

#include <iosfwd>

namespace NPATK {

    class OStreamRuleLogger : public RuleLogger {
    private:
        std::ostream& os;
    public:
        explicit OStreamRuleLogger(std::ostream& stream) : os{stream} { }

        void rule_reduced(const MonomialSubstitutionRule& old_rule,
                          const MonomialSubstitutionRule& new_rule) override;

        void rule_removed(const MonomialSubstitutionRule& ex_rule) override;

        void rule_introduced(
                const MonomialSubstitutionRule& parent_rule_a,
                const MonomialSubstitutionRule& parent_rule_b,
                const MonomialSubstitutionRule& new_rule) override;

        void rule_introduced_conjugate(
                const MonomialSubstitutionRule& parent_rule,
                const MonomialSubstitutionRule& new_rule) override;

        void success(const RuleBook& rb, size_t attempts) override;

        void failure(const RuleBook& rb, size_t attempts) override;
    };
}