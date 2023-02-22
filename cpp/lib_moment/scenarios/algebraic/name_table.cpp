/**
 * name_table.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "name_table.h"

#include <regex>
#include <sstream>
#include <stdexcept>

namespace Moment::Algebraic {

    NameTable::NameTable(std::vector<std::string> &&input_names) :
        operator_count(input_names.size()), names{std::move(input_names)} {
        size_t op_number = 0;
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
}