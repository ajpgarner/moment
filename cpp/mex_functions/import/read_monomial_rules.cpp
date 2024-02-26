/**
 * read_monomial_rules.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "read_monomial_rules.h"

#include "errors.h"

#include "utilities/io_parameters.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

#include "shortlex_hasher.h"
#include "utilities/utf_conversion.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/operator_rule.h"
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
                    throw BadParameter{err.str()};
                }

                try {
                    // Convert from UTF16 -> UTF8
                    std::string utf8str = convertor(mlStr);

                    output.emplace_back(names.find(utf8str));
                } catch (const std::invalid_argument& iae) {
                    std::stringstream err;
                    err << field_name << " cannot be parsed: " << iae.what();
                    throw BadParameter{err.str()};
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
                throw BadParameter{err.str()};
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
                    throw BadParameter{err.str()};
                }
            }

            return output;
        }

        inline std::pair<std::vector<oper_name_t>, bool>
        get_op_seq_from_numeric(matlab::engine::MATLABEngine &matlabEngine,
                                const std::string &field_name,
                                const matlab::data::Array &input,
                                const Algebraic::AlgebraicPrecontext& apc,
                                const bool matlabIndices) {
            std::vector<oper_name_t> output = read_integer_array<oper_name_t>(matlabEngine, field_name, input);
            if (matlabIndices) {
                // Special case 'zero'
                if ((output.size() == 1) && (output[0] == 0)) {
                    return {std::vector<oper_name_t>{}, true};
                }

                for (auto& x : output) {
                    --x;
                }
            }

            for (const auto x: output) {
                if ((x < 0) || ((apc.num_operators != 0) && (x >= apc.num_operators))) {
                    std::stringstream err;
                    err << field_name << " contains an operator with out of bounds value \"" << x << "\"";
                    throw BadParameter{err.str()};
                }
            }
            return {output, false};
        }

        inline std::pair<std::vector<oper_name_t>, bool>
        get_op_seq(matlab::engine::MATLABEngine &matlabEngine,
                  const UTF16toUTF8Convertor& convertor,
                  const std::string &field_name,
                  const matlab::data::Array &input,
                  const Algebraic::AlgebraicPrecontext& apc,
                  const Algebraic::NameTable& names,
                  const bool matlabIndices) {

            // Parse as named operator string
            if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
                auto strArray = static_cast<matlab::data::TypedArray<matlab::data::MATLABString>>(input);
                return {get_op_seq_from_string(matlabEngine, convertor, field_name, strArray, apc, names), false};
            }

            // Parse as one long string.
            if (input.getType() == matlab::data::ArrayType::CHAR) {
                auto charArray = static_cast<matlab::data::CharArray>(input);
                return {get_op_seq_from_char_array(matlabEngine, field_name, charArray, apc, names), false};
            }

            // Otherwise, parse as integer
            return get_op_seq_from_numeric(matlabEngine, field_name, input, apc, matlabIndices);
        }

    }

    Moment::Algebraic::OperatorRule RawMonomialRule::to_rule(matlab::engine::MATLABEngine &matlabEngine,
                                                             const Algebraic::AlgebraicPrecontext &apc,
                                                             const size_t index) const {
        const auto max_strlen = apc.hasher.longest_hashable_string();

        size_t rule_index = 0;
        if (this->LHS.size() > max_strlen) {
            std::stringstream errSS;
            errSS << "Error with rule #" + std::to_string(rule_index+1) + ": LHS too long.";
            throw BadParameter{errSS.str()};
        }
        if (this->RHS.size() > max_strlen) {
            std::stringstream errSS;
            errSS << "Error with rule #" + std::to_string(rule_index+1) + ": RHS too long.";
            throw BadParameter{errSS.str()};
        }

        const auto lhs_hash = apc.hasher.hash(this->LHS);
        const auto rhs_hash = apc.hasher.hash(this->RHS);

        try {
            if (this->rhs_zero) {
                return Algebraic::OperatorRule{
                        HashedSequence{sequence_storage_t(this->LHS.begin(), this->LHS.end()), apc.hasher},
                        HashedSequence{true}};
            }

            if (lhs_hash > rhs_hash) {
                return Algebraic::OperatorRule{
                    HashedSequence{sequence_storage_t(this->LHS.begin(), this->LHS.end()), apc.hasher},
                    HashedSequence{sequence_storage_t(this->RHS.begin(), this->RHS.end()), apc.hasher, this->rule_sign}};
            } else {
                return Algebraic::OperatorRule{
                        HashedSequence{sequence_storage_t(this->RHS.begin(), this->RHS.end()), apc.hasher},
                        HashedSequence{sequence_storage_t(this->LHS.begin(), this->LHS.end()), apc.hasher,
                                       this->rule_sign}};
            }
        } catch (Moment::Algebraic::errors::invalid_rule& ire) {
            std::stringstream errSS;
            errSS << "Error with rule #" + std::to_string(rule_index+1) + ": " << ire.what();
            throw BadParameter{errSS.str()};
        }
    }

    std::vector<RawMonomialRule> read_monomial_rules(matlab::engine::MATLABEngine &matlabEngine,
                                                     matlab::data::Array& input, const std::string& paramName,
                                                     bool matlabIndices,
                                                     const Algebraic::AlgebraicPrecontext& apc,
                                                     const Algebraic::NameTable& names) {

        if (input.getType() != matlab::data::ArrayType::CELL) {
            throw BadParameter{paramName + " must be specified as a cell array."};
        }
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t rule_count =  cell_input.getNumberOfElements();

        const UTF16toUTF8Convertor convertor{};

        std::vector<RawMonomialRule> output;
        output.reserve(rule_count);

        size_t rule_index = 0;
        for (auto elem : cell_input) {
            if (elem.getType() != matlab::data::ArrayType::CELL) {
                throw BadParameter{paramName
                    + " must be specified as a cell array of cell arrays (each with two elements)."};
            }

            const auto rule_cell = static_cast<matlab::data::CellArray>(elem);

            SequenceSignType rule_sign = SequenceSignType::Positive;
            if (elem.getNumberOfElements() == 3) {
                auto mid = rule_cell[1];
                switch (mid.getType()) {
                    case matlab::data::ArrayType::CHAR: {
                        matlab::data::CharArray midAsCA = mid;
                        std::string midVal = midAsCA.toAscii();
                        if (midVal == "-") {
                            rule_sign = SequenceSignType::Negative;
                        } else if (midVal == "i") {
                            rule_sign = SequenceSignType::Imaginary;
                        } else if (midVal == "-i") {
                            rule_sign = SequenceSignType::NegativeImaginary;
                        } else if (midVal == "+") {
                            rule_sign = SequenceSignType::Positive;
                        } else {
                            std::stringstream errSS;
                            errSS << "Each rule must be specified as a cell array of the form {[LHS], [RHS]} or "
                                  << "{[LHS], [sign], [RHS]} where [sign] is one of '+', '-', 'i', '-i'.";
                            throw BadParameter{errSS.str()};
                        }
                    }
                    break;
                    default:
                        throw BadParameter{
                            std::string("Each rule must be specified as a cell array of the form {[LHS], [RHS]} or ")
                            + "{[LHS], '-', [RHS]}; but the middle element provided was not a character array."
                        };
                }
            } else if (elem.getNumberOfElements() != 2) {
                throw BadParameter{
                    std::string("Each rule must be specified as a cell array of the form {[LHS], [RHS]} or ")
                    + "{[LHS], '-', [RHS]}"
                };
            }

            auto lhs = rule_cell[0];
            auto [lhs_rule, lhs_zero] = get_op_seq(matlabEngine, convertor, "Rule #" + std::to_string(rule_index+1) + " LHS",
                                             lhs, apc, names, matlabIndices);

            if (lhs_zero) {
                throw BadParameter{"The LHS of a rule should not be zero."};
            }

            auto rhs = rule_cell[elem.getNumberOfElements() - 1];
            auto [rhs_rule, rhs_zero] = get_op_seq(matlabEngine, convertor, "Rule #" + std::to_string(rule_index+1) + " RHS",
                                             rhs, apc, names, matlabIndices);
            if (rhs_zero) {
                rule_sign = SequenceSignType::Positive;
            }

            output.emplace_back(std::move(lhs_rule), std::move(rhs_rule), rule_sign, rhs_zero);
            ++rule_index;
        }
        return output;
    }


    void throw_bad_length(matlab::engine::MATLABEngine &matlabEngine,
                          const std::vector<oper_name_t>& vec, size_t n, const std::string& lhs_or_rhs) {
        std::stringstream ss;
        ss << "Rule number #" << n << " " << lhs_or_rhs << " is too long.";
        throw BadParameter{ss.str()};
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