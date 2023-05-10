/**
 * read_monomial_rules.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "read_monomial_rules.h"

#include "error_codes.h"

#include "utilities/io_parameters.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

#include "utilities/shortlex_hasher.h"
#include "utilities/utf_conversion.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/name_table.h"

#include <stdexcept>

namespace Moment::mex {
    namespace {

        std::vector<oper_name_t>
        get_op_seq_from_string(matlab::engine::MATLABEngine &matlabEngine,
                               const UTF16toUTF8Convertor& convertor,
                               const std::string &field_name,
                               const matlab::data::TypedArray<matlab::data::MATLABString> &input,
                               const Algebraic::AlgebraicPrecontext& apc,
                               const Algebraic::NameTable& names) {
            std::vector<oper_name_t> output;
            output.reserve(input.getNumberOfElements());

            for (const auto& mlStr : input) {
                if (!mlStr.has_value()) {
                    std::stringstream err;
                    err << field_name << " cannot be parsed as an operator sequence, as it contains an empty string.";
                    throw errors::BadInput{errors::bad_param, err.str()};
                }

                try {
                    // Convert from UTF16 -> UTF8
                    std::string utf8str = convertor(mlStr);

                    output.emplace_back(names.find(utf8str));
                } catch (const std::invalid_argument& iae) {
                    std::stringstream err;
                    err << field_name << " cannot be parsed: " << iae.what();
                    throw errors::BadInput{errors::bad_param, err.str()};
                }
            }
            return output;
        }

        std::vector<oper_name_t>
        get_op_seq_from_char_array(matlab::engine::MATLABEngine &matlabEngine,
                                   const std::string &field_name,
                                   const matlab::data::CharArray &input,
                                   const Algebraic::AlgebraicPrecontext& apc,
                                   const Algebraic::NameTable& names) {
            // Validate names object
            if (!names.all_single()) {
                std::stringstream err;
                err << field_name
                    << " can only be parsed as a char array when every operator name is a single character.";
                throw errors::BadInput{errors::bad_param, err.str()};
            }

            auto asString = input.toAscii();

            std::vector<oper_name_t> output;
            output.reserve(asString.length());

            for (auto one_char : asString) {
                try {
                    output.emplace_back(names.find(std::string(1, one_char)));
                } catch (const std::invalid_argument& iae) {
                    std::stringstream err;
                    err << field_name << " cannot be parsed:" << iae.what();
                    throw errors::BadInput{errors::bad_param, err.str()};
                }
            }

            return output;
        }

        std::vector<oper_name_t>
        get_op_seq_from_numeric(matlab::engine::MATLABEngine &matlabEngine,
                                const std::string &field_name,
                                const matlab::data::Array &input,
                                const Algebraic::AlgebraicPrecontext& apc,
                                const bool matlabIndices) {
            std::vector<oper_name_t> output = read_integer_array<oper_name_t>(matlabEngine, field_name, input);
            if (matlabIndices) {
                for (auto& x : output) {
                    --x;
                }
            }

            for (const auto x: output) {
                if ((x < 0) || ((apc.num_operators != 0) && (x >= apc.num_operators))) {
                    std::stringstream err;
                    err << field_name << " contains an operator with out of bounds value \"" << x << "\"";
                    throw errors::BadInput{errors::bad_param, err.str()};
                }
            }
            return output;
        }

        std::vector<oper_name_t> get_op_seq(matlab::engine::MATLABEngine &matlabEngine,
                                            const UTF16toUTF8Convertor& convertor,
                                            const std::string &field_name,
                                            const matlab::data::Array &input,
                                            const Algebraic::AlgebraicPrecontext& apc,
                                            const Algebraic::NameTable& names,
                                            const bool matlabIndices) {

            // Parse as named operator string
            if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
                auto strArray = static_cast<matlab::data::TypedArray<matlab::data::MATLABString>>(input);
                return get_op_seq_from_string(matlabEngine, convertor, field_name, strArray, apc, names);
            }

            // Parse as one long string.
            if (input.getType() == matlab::data::ArrayType::CHAR) {
                auto charArray = static_cast<matlab::data::CharArray>(input);
                return get_op_seq_from_char_array(matlabEngine, field_name, charArray, apc, names);
            }

            // Otherwise, parse as integer
            return get_op_seq_from_numeric(matlabEngine, field_name, input, apc, matlabIndices);
        }

    }

    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     bool matlabIndices,
                                                     const Algebraic::AlgebraicPrecontext& apc,
                                                     const Algebraic::NameTable& names) {

        if (input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, paramName + " must be specified as a cell array.");
        }
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t rule_count =  cell_input.getNumberOfElements();


        const UTF16toUTF8Convertor convertor{};

        std::vector<RawMonomialRule> output;
        output.reserve(rule_count);

        size_t rule_index = 0;
        for (auto elem : cell_input) {
            if (elem.getType() != matlab::data::ArrayType::CELL) {
                throw_error(matlabEngine, errors::bad_param,
                            paramName + " must be specified as a cell array of cell arrays (each with two elements).");
            }

            const auto rule_cell = static_cast<matlab::data::CellArray>(elem);

            bool negated = false;
            if (elem.getNumberOfElements() == 3) {
                auto mid = rule_cell[1];
                switch (mid.getType()) {
                    case matlab::data::ArrayType::CHAR: {
                        matlab::data::CharArray midAsCA = mid;
                        std::string midVal = midAsCA.toAscii();
                        negated = (midVal == "-");
                    }
                    default:
                        break;
                }

                // Could not verify negation:
                if (!negated) {
                    throw_error(matlabEngine, errors::bad_param,
                                std::string("Each rule must be specified as a cell array of the form {[LHS], [RHS]} or ")
                                + "{[LHS], '-', [RHS]}");
                }
            } else if (elem.getNumberOfElements() != 2) {
                throw_error(matlabEngine, errors::bad_param,
                            std::string("Each rule must be specified as a cell array of the form {[LHS], [RHS]} or ")
                                      + "{[LHS], '-', [RHS]}");
            }

            auto lhs = rule_cell[0];
            auto lhs_rules = get_op_seq(matlabEngine, convertor, "Rule #" + std::to_string(rule_index+1) + " LHS",
                                             lhs, apc, names, matlabIndices);

            auto rhs = rule_cell[negated ? 2 : 1];
            auto rhs_rules = get_op_seq(matlabEngine, convertor, "Rule #" + std::to_string(rule_index+1) + " RHS",
                                             rhs, apc, names, matlabIndices);

            output.emplace_back(std::move(lhs_rules), std::move(rhs_rules), negated);

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