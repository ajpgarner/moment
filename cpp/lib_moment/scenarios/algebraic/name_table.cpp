/**
 * name_table.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "name_table.h"

#include "algebraic_precontext.h"

#include <regex>
#include <sstream>
#include <stdexcept>

namespace Moment::Algebraic {


    NameTable::NameTable(const AlgebraicPrecontext& apc, std::vector<std::string>&& input_names)
        : operator_count(input_names.size()), names{std::move(input_names)} {
        //const size_t raw_op_count =  (apc.num_operators / (apc.self_adjoint ? 2 : 1));

        // Check number of names matches APC's expected number of names
        if (operator_count != apc.raw_operators) {
            std::stringstream ss;
            ss << operator_count << " " << ((operator_count != 1) ? "names" : "name")
               << " provided, but context expects "
               << apc.raw_operators << ((apc.raw_operators != 1) ? "names" : "name") << ".";
            throw std::invalid_argument{ss.str()};
        }

        // Validate raw names
        oper_name_t op_number = 0;
        this->all_single_char = true;
        for (const auto& name : this->names) {
            const auto tx_op_number = static_cast<oper_name_t>(op_number
                                        * ((apc.conj_mode == AlgebraicPrecontext::ConjugateMode::Interleaved) ? 2 : 1));
            auto result = NameTable::validate_name(name);
            if (result.has_value()) {
                std::stringstream ss;
                ss << "Invalid name for operator " << (op_number+1) << ": " << result.value();
                throw std::invalid_argument{ss.str()};
            }
            auto [iter, new_insert] = this->index.insert(std::make_pair(name, tx_op_number));
            if (!new_insert) {
                std::stringstream ss;
                ss << "Operator #" << (op_number + 1) << " has duplicate name \"" << name << "\""
                   << " (same as operator #" << (iter->second +1) << ")";
                throw std::invalid_argument{ss.str()};
            }
            if (name.length() != 1) {
                this->all_single_char = false;
            }

            ++op_number;
        }

        // Add * variants, if required
        switch (apc.conj_mode) {
            case AlgebraicPrecontext::ConjugateMode::SelfAdjoint:
                for (size_t idx = 0; idx < this->operator_count; ++idx) {
                    std::string conj_str = this->names[idx];
                    conj_str.append("*");
                    this->index.emplace(std::make_pair(std::move(conj_str), idx));
                }
                break;
            case AlgebraicPrecontext::ConjugateMode::Bunched:
                this->names.reserve(2 * this->names.size());
                for (size_t strIndex = 0; strIndex < this->operator_count; ++strIndex) {
                    std::string conj_str = this->names[strIndex];
                    conj_str.append("*");
                    this->names.emplace_back(std::move(conj_str));
                    this->index.emplace(std::make_pair(this->names.back(), this->names.size()-1));
                }
                break;
            case AlgebraicPrecontext::ConjugateMode::Interleaved:
                this->names.resize(2* this->names.size());
                for (ptrdiff_t idx = this->operator_count-1; idx >= 0; --idx) {
                    std::string conj_str = this->names[idx];
                    conj_str.append("*");

                    this->names[2*idx] = std::move(this->names[idx]);

                    this->index.emplace(std::make_pair(conj_str, (2*idx+1)));
                    this->names[2*idx+1] = std::move(conj_str);
                }
                break;
        }
    }

    NameTable::NameTable(const AlgebraicPrecontext& apc)
        : NameTable(apc, default_string_names(apc.raw_operators, "X")) {
    }

    NameTable::NameTable(std::initializer_list<std::string> input_names)
        : NameTable{AlgebraicPrecontext{static_cast<oper_name_t>(input_names.size())},
                    std::vector<std::string>(input_names.begin(), input_names.end())} {

    }

    std::vector<std::string> NameTable::default_string_names(size_t num_operators, const std::string& var_name) {
        if (num_operators <= 0) {
            return {};
        }

        std::vector<std::string> output;
        output.reserve(num_operators);
        for (size_t i = 0; i < num_operators; ++i) {
            std::stringstream ss;
            ss << var_name << (i+1);
            output.emplace_back(ss.str());
        }
        return output;
    }

    std::optional<std::string> NameTable::validate_name(const std::string& name) {
        // Empty string never valid
        if (name.empty()) {
            return {"Name must not be empty string."};
        }

        // Fully valid if matches pattern
        std::regex valid_pattern{"[A-Za-z][A-Za-z0-9_]*"};
        if (std::regex_match(name, valid_pattern)) {
            return std::nullopt;
        }

        // Is there a problem with the first char?
        auto first_char = name.substr(0, 1);
        if (std::regex_match(first_char, std::regex{"[0-9_]"})) {
            return {"Name must begin with a letter."};
        }

        // Otherwise, generic fail:
        return {"Name must be alphanumeric, and begin with a letter."};
    }

    oper_name_t NameTable::find(const std::string& str) const {
        if (str.empty()) {
            throw std::invalid_argument{"Operator cannot be empty string."};
        }

        auto search_iter = this->index.find(str);

        if (search_iter == this->index.end()) {
            std::stringstream errSS;
            errSS << "Cannot find operator \"" << str << "\"";
            throw std::invalid_argument{errSS.str()};
        }

        // Names now stored in order.
        return search_iter->second;
    }


}