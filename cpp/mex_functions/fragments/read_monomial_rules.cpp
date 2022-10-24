/**
 * read_monomial_rules.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "read_monomial_rules.h"

#include "error_codes.h"

#include "utilities/io_parameters.h"
#include "utilities/reporting.h"

#include "operators/shortlex_hasher.h"

namespace NPATK::mex {
    namespace {
        std::vector<oper_name_t> getBoundedRules(matlab::engine::MATLABEngine &matlabEngine,
                                                 const std::string &name,
                                                 const matlab::data::Array &input,
                                                 uint64_t operator_count) {
            std::vector<oper_name_t> output = SortedInputs::read_integer_array(matlabEngine, name, input);
            for (const auto x: output) {
                if ((x < 0) || ((operator_count != 0) && (x >= operator_count))) {
                    std::stringstream err;
                    err << name << " contains an operator with out of bounds value \"" << x << "\"";
                    throw errors::BadInput{errors::bad_param, err.str()};
                }
            }
            return output;
        }

    }

    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     uint64_t operator_bound) {

        if (input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, paramName + " must be specified as a cell array.");
        }
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t rule_count =  cell_input.getNumberOfElements();

        std::vector<RawMonomialRule> output;
        output.reserve(rule_count);

        size_t rule_index = 0;
        for (auto elem : cell_input) {
            if (elem.getType() != matlab::data::ArrayType::CELL) {
                throw_error(matlabEngine, errors::bad_param,
                            paramName + " must be specified as a cell array of cell arrays (each with two elements).");
            }
            if (elem.getNumberOfElements() != 2) {
                throw_error(matlabEngine, errors::bad_param,
                            "Each rule must be specified as a cell array with two elements.");
            }

            const auto rule_cell = static_cast<matlab::data::CellArray>(elem);
            auto lhs = rule_cell[0];
            auto lhs_rules = getBoundedRules(matlabEngine, "Rule #" + std::to_string(rule_index+1) + " LHS",
                                             lhs, operator_bound);

            auto rhs = rule_cell[1];
            auto rhs_rules = getBoundedRules(matlabEngine, "Rule #" + std::to_string(rule_index+1) + " RHS",
                                             rhs, operator_bound);

            output.emplace_back(std::move(lhs_rules), std::move(rhs_rules));

            ++rule_index;
        }
        return output;
    }

    void throw_bad_length(matlab::engine::MATLABEngine &matlabEngine,
                          const std::vector<oper_name_t>& vec, size_t n, const std::string& lhs_or_rhs) {
        std::stringstream ss;
        ss << "Rule number #" << n << " " << lhs_or_rhs << " is too long.";

        throw_error(matlabEngine, errors::bad_param, ss.str());
    }

    void check_rule_length(matlab::engine::MATLABEngine &matlabEngine,
                           const ShortlexHasher &hasher, const std::vector<RawMonomialRule> &raw) {
        size_t n = 1;
        const size_t max_strlen = hasher.longest_hashable_string();
        for (auto& ir : raw) {
            if (ir.LHS.size() > max_strlen) {
                throw_bad_length(matlabEngine, ir.LHS, n, "LHS");
            }
            if (ir.RHS.size() > max_strlen) {
                throw_bad_length(matlabEngine, ir.RHS, n, "RHS");
            }
            ++n;
        }
    }

}