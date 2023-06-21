/**
 * moment_rule_helpers.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "gtest/gtest.h"

#include "symbolic/moment_rule.h"
#include "symbolic/moment_rulebook.h"

#include <string>
#include <vector>

namespace Moment::Tests {

    inline void assert_matching_rules(const MomentRulebook &book,
                               const std::vector<MomentRule> &expected) {
        ASSERT_EQ(book.size(), expected.size());
        ASSERT_EQ(book.empty(), expected.empty());

        size_t rule_number = 0;
        auto expected_iter = expected.begin();
        for (const auto &[id, rule]: book) {
            EXPECT_EQ(id, expected_iter->LHS()) << "Rule #" << rule_number;
            EXPECT_EQ(rule.LHS(), expected_iter->LHS()) << "Rule #" << rule_number;
            EXPECT_EQ(rule.RHS(), expected_iter->RHS()) << "Rule #" << rule_number;
            ++expected_iter;
            ++rule_number;
        }
    }


    inline void expect_matching_polynomials(const std::string& label,
                                            const Polynomial& LHS, const Polynomial& RHS, double tolerance) {
        EXPECT_TRUE(LHS.approximately_equals(RHS, tolerance))
            << label << (label.empty() ? "" : "\n") << "LHS = \n" << LHS << "\n RHS = \n" << RHS;
    }


    inline void expect_matching_rule(const std::string& label,
                                     const MomentRule& lhs,
                                     const MomentRule& rhs,
                                     double zero_tolerance) {
        EXPECT_EQ(lhs.is_partial(), rhs.is_partial()) << label;
        EXPECT_EQ(lhs.LHS(), rhs.LHS()) << label;
        EXPECT_TRUE(approximately_equal(lhs.partial_direction(), rhs.partial_direction(), zero_tolerance))
                            << label << (label.empty() ? "" : " ") << "direction,\n"
                            << "LHS = " << lhs.partial_direction() << ",\nRHS = " << rhs.partial_direction();

        std::string new_label{label};
        new_label.append(" (RHS)");
        expect_matching_polynomials(new_label, lhs.RHS(), rhs.RHS(), zero_tolerance);
    }
}