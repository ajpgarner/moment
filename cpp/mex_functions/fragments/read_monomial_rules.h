/**
 * read_monomial_rules.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "integer_types.h"

#include <vector>

namespace NPATK {
    class ShortlexHasher;
}

namespace NPATK::mex {

    struct RawMonomialRule {
        std::vector<oper_name_t> LHS{};
        std::vector<oper_name_t> RHS{};

        RawMonomialRule() = default;

        RawMonomialRule(std::vector<oper_name_t> lhs, std::vector<oper_name_t> rhs)
                : LHS(std::move(lhs)), RHS(std::move(rhs)) { }

    };


    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     uint64_t operator_bound = 0);

    void check_rule_length(matlab::engine::MATLABEngine &matlabEngine,
                           const ShortlexHasher& hasher, const std::vector<RawMonomialRule>& raw);


}