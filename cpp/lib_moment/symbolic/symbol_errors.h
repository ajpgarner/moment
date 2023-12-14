/**
 * symbol_errors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"

#include <stdexcept>
#include <string>


namespace Moment::errors {
    /**
     * Error: Some inference makes a symbol algebraically always zero, when that symbol is not ID 0.
     */
    class zero_symbol : public std::runtime_error {
    public:
        /** The requested symbol ID */
        const symbol_name_t id;

        explicit zero_symbol(symbol_name_t id);
    };

    /**
     * Error: Symbol with particular ID not found in symbol table.
     */
    class unknown_symbol : public std::domain_error {
    public:
        /** The requested symbol ID */
        const symbol_name_t id;

        explicit unknown_symbol(symbol_name_t id);
    };

    /**
     * Error: Symbol at particular ID does not have a defined basis element of a particular type.
     */
    class unknown_basis_elem : public std::domain_error {
    public:
        /** The requested basis element index */
        const ptrdiff_t id;

        /** True if the real basis element was requested, false if the imaginary basis element was requested. */
        const bool real;
        unknown_basis_elem(bool is_real, ptrdiff_t id);
    };

    /**
     * Error: Operator sequence does not correspond to an entry in a symbol table, but a match was expected/required.
     */
    class unregistered_operator_sequence : public std::runtime_error {
    public:
        /** The hash of the missing operator sequence. */
        const uint64_t missing_hash;

        unregistered_operator_sequence(const std::string& formatted_sequence, uint64_t hash);
    };

}