/**
 * monomial.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "monomial.h"

#include <cassert>

#include <iostream>

#include "scenarios/context.h"
#include "scenarios/contextual_os_helper.h"
#include "symbol_table.h"

#include "utilities/format_factor.h"

namespace Moment {

    std::string Monomial::as_string(const StringFormatContext &format_context) const {
        return make_contextualized_string(format_context, [this](ContextualOS& os) {
            os << *this;
        });
    }

    void Monomial::format_as_symbol_id_without_context(std::ostream& os, bool show_plus, bool show_hash) const {
        // Zero
        if ((this->id == 0) || (this->factor == 0.0)) { // not approx, actual!
            if (show_plus) {
                os << " + ";
            }
            os << "0";
            return;
        }

        const bool is_scalar = (this->id == 1);
        const bool needs_space = format_factor(os, this->factor, is_scalar, show_plus);

        if (!is_scalar) {
            if (needs_space) {
                if (show_hash) {
                    os << " ";
                } else {
                    os << "*";
                }
            }

            if (show_hash) {
                os << "#";
            }
            os << this->id;
            if (this->conjugated) {
                os << "*";
            }
        }
    }

    void Monomial::format_as_symbol_id_with_context(ContextualOS &os) const {
        // Zero
        if ((this->id == 0) || (this->factor == 0.0)) { // not approx, actual!
            if (!os.format_info.first_in_polynomial) {
                os << " + ";
            }
            os << "0";
            return;
        }

        const bool is_scalar = (this->id == 1);
        const bool needs_space = format_factor(os, this->factor, is_scalar, !os.format_info.first_in_polynomial);

        if (!is_scalar) {
            if (needs_space) {
                switch (os.format_info.prefactor_join) {
                    case StringFormatContext::PrefactorJoin::Space:
                        os.os << " ";
                        break;
                    case StringFormatContext::PrefactorJoin::Asterix:
                        os.os << "*";
                        break;
                    case StringFormatContext::PrefactorJoin::Nothing:
                        break;
                }
            }

            if (os.format_info.hash_before_symbol_id) {
                os << "#";
            }
            os << this->id;
            if (this->conjugated) {
                os << "*";
            }
        }
    }

    void Monomial::format_as_operator_sequence_with_context(ContextualOS& os) const {
        if constexpr(debug_mode) {
            if (os.symbols == nullptr) [[unlikely]] {
                throw std::runtime_error{"Symbol table must be supplied to contextual OS for OS output."};
            }
        }

        // Zero
        if ((this->id == 0) || (this->factor == 0.0)) { // not approx, actual!
            if (!os.format_info.first_in_polynomial) {
                os << " + ";
            }
            os << "0";
            return;
        }

        // Is element a scalar?
        const bool is_scalar = (this->id == 1);

        // Write factor
        const bool need_space = format_factor(os.os, this->factor, is_scalar, !os.format_info.first_in_polynomial);

        // Scalar, factor alone is enough
        if (is_scalar) {
            return;
        }

        if (need_space) {
            switch (os.format_info.prefactor_join) {
                case StringFormatContext::PrefactorJoin::Space:
                    os.os << " ";
                    break;
                case StringFormatContext::PrefactorJoin::Asterix:
                    os.os << "*";
                    break;
                case StringFormatContext::PrefactorJoin::Nothing:
                    break;
            }
        }

        // Skip if symbol not in table.
        const auto& symbols = *os.symbols;
        const bool valid_symbol = ((this->id >= 0) && (this->id < symbols.size()));
        if (!valid_symbol) {
            os.os << "UNK#" << this->id;
            return;
        }

        // Get symbol information
        const auto &symbol_info = symbols[this->id];

        // Is symbol associated with operator sequence?
        if (symbol_info.has_sequence()) {
            if (this->conjugated) {
                os.context.format_sequence(os, symbol_info.sequence_conj());
            } else {
                os.context.format_sequence(os, symbol_info.sequence());
            }
        } else { // Otherwise, fall back to other OS information
            os.context.format_sequence_from_symbol_id(os, this->id, this->conjugated);
        }
    }


    std::ostream& operator<<(std::ostream& os, const Monomial& expr) {
        // Get flags
        const bool show_plus = os.flags() & std::ios::showpos;
        os.unsetf(std::ios::showpos);
        const bool show_hash = os.flags() & std::ios::showbase;
        os.unsetf(std::ios::showbase);

        // Do output
        expr.format_as_symbol_id_without_context(os, show_plus, show_hash);

        // Reset flags
        if (show_plus) {
            os.setf(std::ios::showpos);
        }
        if (show_hash) {
            os.setf(std::ios::showbase);
        }

        return os;
    }

    ContextualOS& operator<<(ContextualOS& os, const Monomial& expr) {
        // If we don't have the symbol table, default to normal << mode
        if ((os.symbols == nullptr) || (os.format_info.display_symbolic_as == ContextualOS::DisplayAs::SymbolIds)) {
            expr.format_as_symbol_id_with_context(os);
            return os;
        }

        // Switch format mode
        assert(os.format_info.display_symbolic_as == ContextualOS::DisplayAs::Operators);
        expr.format_as_operator_sequence_with_context(os);

        return os;
    }

    Monomial::Monomial(const std::string& strExpr) {
        // Size must be in bounds
        if (strExpr.empty() || (strExpr.size() > Monomial::max_strlen)) {
            throw SymbolParseException{strExpr};
        }
        size_t read_from = 0;
        size_t read_to = strExpr.length();

        // Test if monomial includes a prefactor with #
        size_t prefactor_split = strExpr.find('#');
        bool has_prefactor = (prefactor_split != std::string::npos);
        bool just_a_number = false;
        if (!has_prefactor) {
            // No explicit prefactor, but number is a double:
            if (strExpr.find('.') != std::string::npos) {
                has_prefactor = true;
                prefactor_split = read_to;
                just_a_number = true;
            }
        }

        // Attempt to read pre-factor
        if (has_prefactor) {
            read_from = prefactor_split+1;
            if (prefactor_split > 0) {
                try {
                    size_t how_much = 0;
                    this->factor = std::stod(strExpr.substr(0, prefactor_split), &how_much);
                    if (how_much < prefactor_split) {
                        throw SymbolParseException{strExpr};
                    }
                } catch (std::invalid_argument &e) {
                    throw SymbolParseException{strExpr, e};
                } catch (std::out_of_range &e) {
                    throw SymbolParseException{strExpr, e};
                }
            } else {
                this->factor = 1.0;
            }
        } else {
            this->factor = 1.0;
        }

        // Prefactor only
        if (just_a_number) {
            this->conjugated = false;
            this->id = 1;
            return;
        }

        // Test if conjugate string
        if (strExpr.ends_with('*')) {
            this->conjugated = true;
            --read_to;
        } else {
            this->conjugated = false;
        }

        // Attempt to read symbol ID
        try {
            size_t how_much = 0;
            auto read_symbol_id = std::stoi(strExpr.substr(read_from, read_to - read_from), &how_much);

            // Make sure whole string is read
            if (read_from + how_much < read_to) {
                throw SymbolParseException{strExpr};
            }

            if (read_symbol_id < 0) {
                if (!has_prefactor) {
                    this->factor = -1.0;
                    read_symbol_id = -read_symbol_id;
                } else {
                    // Negative and prefactor not allowed...!
                    throw SymbolParseException{strExpr};
                }
            }
            this->id = static_cast<symbol_name_t>(read_symbol_id);
        } catch (std::invalid_argument& e) {
            throw SymbolParseException{strExpr, e};
        } catch (std::out_of_range& e) {
            throw SymbolParseException{strExpr, e};
        }
    }


    std::string Monomial::SymbolParseException::make_msg(const std::string &badExpr) {
        if (badExpr.length() > Monomial::max_strlen) {
            return std::string("Could not parse \"" + badExpr.substr(0, Monomial::max_strlen) + "...\" as a symbol.");
        }
        return std::string("Could not parse \"" + badExpr + "\" as a symbol.");
    }

    std::string Monomial::SymbolParseException::make_msg(const std::string &badExpr, const std::exception &e) {
        if (badExpr.length() > Monomial::max_strlen) {
            return std::string("Could not parse \"" + badExpr.substr(0, Monomial::max_strlen) + "...\" as a symbol."
                               + "\nThe following exception occurred: " + e.what());
        }
        return std::string("Could not parse \"" + badExpr + "\" as a symbol.\nThe following exception occurred: "
                           + e.what());
    }
}