/**
 * ostream_rule_logger.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "rule_book.h"

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

        void rule_reduced(const MonomialSubstitutionRule& old_rule,
                          const MonomialSubstitutionRule& new_rule) override;

        void rule_removed(const MonomialSubstitutionRule& ex_rule) override;

        void rule_introduced(const MonomialSubstitutionRule& new_rule) override;

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