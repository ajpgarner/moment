/**
 * name_table.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include <cassert>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Moment::Algebraic {

    class AlgebraicPrecontext;

    class NameTable {
    public:
        const size_t operator_count;
    private:
        std::vector<std::string> names;
        std::map<std::string, oper_name_t> index;

        bool all_single_char = false;

    public:
        /**
         * Create table of names.
         * @param names The names of each fundamental operator, in order.
         * @throws std::invalid_argument if any name is invalid, or if there is a duplicate name.
         */
        explicit NameTable(std::vector<std::string>&& names);

        /**
         * Create table of automatically generated names.
         * @param num_operators The number of fundamental operators.
         */
        explicit NameTable(size_t num_operators) : NameTable(default_string_names(num_operators)) { }

        /** Gets the name associated with the operator at index. */
        inline const std::string& operator[](const size_t idx) const noexcept {
            assert(idx < this->names.size());
            return this->names[idx];
        }

        /** True, if every name is just one letter long */
        bool all_single() const noexcept { return this->all_single_char; }

        /**
         * Translate name to operator number.
         * @param apc The algebraic pre-context.
         * @param str The name to attempt to match.
         * @throws std::invalid_argument If string cannot be translated to valid operator number.
         */
        [[nodiscard]] oper_name_t find(const AlgebraicPrecontext& apc, std::string str) const;

    public:
        /** Checks if a name is allowed.
         * @param name The name to test.
         * @return Empty optional if valid, otherwise the reason for rejection
         */
        static std::optional<std::string> validate_name(const std::string& name);

    private:
        static std::vector<std::string> default_string_names(size_t num_operators, const std::string& var_name = "X");

    };

}