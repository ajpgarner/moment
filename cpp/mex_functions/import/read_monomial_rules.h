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
    namespace Algebraic {
        class AlgebraicPrecontext;
        class OperatorRule;
        class NameTable;
    }
}

namespace Moment::mex {

    struct RawMonomialRule {
        std::vector<oper_name_t> LHS{};
        std::vector<oper_name_t> RHS{};
        bool negated = false;

        RawMonomialRule() = default;

        RawMonomialRule(std::vector<oper_name_t> lhs, std::vector<oper_name_t> rhs, bool neg)
                : LHS(std::move(lhs)), RHS(std::move(rhs)), negated{neg} { }

        /**
         * Orient and hash rule.
         * @param matlabEngine MATLAB engine, for errors
         * @param apc AlgebraicPrecontext, for hashing and max length.
         * @param index Index, for error messages.
         * @return Oriented and hashed rule.
         */
        Moment::Algebraic::OperatorRule to_rule(matlab::engine::MATLABEngine &matlabEngine,
                                                const Algebraic::AlgebraicPrecontext& apc,
                                                const size_t index) const;
    };


    /**
     * Reads a cell array of cell-array pairs, and parse it into a vector of RawMonomialRule.
     */
    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     bool matlab_indices,
                                                     const Algebraic::AlgebraicPrecontext& apc,
                                                     const Algebraic::NameTable& names);


    void check_rule_length(matlab::engine::MATLABEngine &matlabEngine,
                           const ShortlexHasher& hasher, const std::vector<RawMonomialRule>& raw);


}