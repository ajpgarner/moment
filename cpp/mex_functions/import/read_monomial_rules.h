/**
 * read_monomial_rules.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "integer_types.h"

#include <vector>

namespace Moment {
    class ShortlexHasher;
}

namespace Moment::mex {

    struct RawMonomialRule {
        std::vector<oper_name_t> LHS{};
        std::vector<oper_name_t> RHS{};
        bool negated = false;

        RawMonomialRule() = default;

        RawMonomialRule(std::vector<oper_name_t> lhs, std::vector<oper_name_t> rhs, bool neg)
                : LHS(std::move(lhs)), RHS(std::move(rhs)), negated{neg} { }

    };


    /**
     * Reads a cell array of cell-array pairs, and parse it into a vector of RawMonomialRule.
     */
    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     bool matlab_indices,
                                                     uint64_t operator_bound = 0);

    void check_rule_length(matlab::engine::MATLABEngine &matlabEngine,
                           const ShortlexHasher& hasher, const std::vector<RawMonomialRule>& raw);


}