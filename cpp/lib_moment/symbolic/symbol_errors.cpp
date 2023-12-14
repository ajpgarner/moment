/**
 * symbol_errors.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol_errors.h"
#include <sstream>

namespace Moment::errors {
    namespace {
        [[nodiscard]] std::string make_zs_err_msg(const symbol_name_t id) {
            std::stringstream errMsg;
            errMsg << "Symbol " << id << " is identically zero; but zero should be uniquely represented as \"0\"";
            return errMsg.str();
        }

        [[nodiscard]] std::string make_us_err_msg(const symbol_name_t id) {
            std::stringstream errMsg;
            errMsg << "Symbol " << id << " is not defined in symbol table.";
            return errMsg.str();
        }

        [[nodiscard]] std::string make_ube_err_msg(const bool real, const ptrdiff_t id) {
            std::stringstream errMsg;
            if (real) {
                errMsg << "Real";
            } else {
                errMsg << "Imaginary";
            }
            errMsg << " basis element " << id << " is not defined in symbol table.";
            return errMsg.str();
        }

        [[nodiscard]] std::string make_uos_err_msg(const std::string& formatted_sequence, const uint64_t hash_num) {
            std::stringstream errMsg;
            errMsg << "Sequence '" << formatted_sequence << "' (hash: " << hash_num << ") "
                   << "did not correspond to an entry in the symbol table.";
            return errMsg.str();
        }
    }

    zero_symbol::zero_symbol(symbol_name_t sid) : std::runtime_error{make_zs_err_msg(sid)}, id{sid} { }

    unknown_symbol::unknown_symbol(Moment::symbol_name_t sid) : std::domain_error{make_us_err_msg(sid)}, id{sid} { }

    unknown_basis_elem::unknown_basis_elem(bool real_or, ptrdiff_t basis_id)
            : std::domain_error{make_ube_err_msg(real_or, basis_id)}, real{real_or}, id{basis_id} { }

    unregistered_operator_sequence::unregistered_operator_sequence(const std::string& formatted_sequence,
                                                                   const uint64_t hash)
            : runtime_error(make_uos_err_msg(formatted_sequence, hash)), missing_hash{hash} { }

}