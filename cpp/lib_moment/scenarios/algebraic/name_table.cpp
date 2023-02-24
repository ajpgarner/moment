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

    NameTable::NameTable(std::vector<std::string> &&input_names) :
        operator_count(input_names.size()), names{std::move(input_names)} {
        oper_name_t op_number = 0;
        this->all_single_char = true;
        for (const auto& name : this->names) {
            auto result = NameTable::validate_name(name);
            if (result.has_value()) {
                std::stringstream ss;
                ss << "Invalid name for operator " << (op_number+1) << ": " << result.value();
                throw std::invalid_argument{ss.str()};
            }
            auto [iter, new_insert] = this->index.insert(std::make_pair(name, op_number));
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

        // Add "*" variants to names vector.
        this->names.reserve(2*this->names.size());
        for (size_t strIndex = 0, index_max = this->names.size(); strIndex < index_max; ++strIndex) {
            this->names.emplace_back(this->names[strIndex]);
            this->names.back().append("*");
        }
    }

    std::vector<std::string> NameTable::default_string_names(size_t num_operators, const std::string& var_name) {
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

    oper_name_t NameTable::find(const AlgebraicPrecontext& apc, std::string str) const {
        if (str.empty()) {
            throw std::invalid_argument{"Operator cannot be empty string."};
        }

        // Is operator conjugated?
        bool conjugated = false;
        if (str.ends_with('*')) {
            str = str.substr(0, str.size()-1);
            conjugated = !apc.self_adjoint;
        }

        // Try and match name
        auto search_iter = this->index.find(str);
        if (search_iter == this->index.end()) {
            std::stringstream errSS;
            errSS << "Cannot find operator \"" << str << "\"";
            throw std::invalid_argument{errSS.str()};
        }

        // Extract operator name, apply conjugation if necessary
        oper_name_t raw_oper = search_iter->second;
        if (conjugated) {
            raw_oper += apc.conj_offset;
        }

        return raw_oper;
    }
}