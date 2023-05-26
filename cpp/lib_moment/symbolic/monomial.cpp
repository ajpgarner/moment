/**
 * monomial.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "monomial.h"

#include <iostream>
#include <sstream>

namespace Moment {

    void Monomial::format_factor(std::ostream& os, std::complex<double> factor, bool mandatory_plus) {
        if (approximately_real(factor)) {
            if (mandatory_plus) {
                if (factor.real() > 0) {
                    os << " + " << factor.real();
                } else {
                    os << " - " << (-factor.real());
                }
            } else {
                os << factor.real();
            }
        } else if (approximately_imaginary(factor)) {
            if (mandatory_plus) {
                if (factor.imag() > 0) {
                    os << " + " << factor.imag() << "i";
                } else {
                    os << " - " << (-factor.imag()) << "i";
                }
            } else {
                os << factor.imag() << "i";
            }
        } else { // Complex number
            if (mandatory_plus) {
                os << " + ";
            }
            os << "(" << factor.real() << " + " << factor.imag() << "i)";
        }
    }

    void Monomial::format_factor_skip_one(std::ostream& os, std::complex<double> factor,
                                          bool mandatory_plus, bool include_times) {
        if (approximately_equal(factor, 1.0)) { // +1
            if (mandatory_plus) {
                os << " + ";
            }
        } else if (approximately_equal(factor, -1.0)) { // -1
            if (mandatory_plus) {
                os << " - ";
            } else {
                os << "-";
            }
        } else { // General factor
            format_factor(os, factor, mandatory_plus);
            if (include_times) {
                os << "*";
            }
        }
    }

    std::ostream& operator<<(std::ostream& os, const Monomial& expr) {

        const bool show_plus = os.flags() & std::ios::showpos;
        os.unsetf(std::ios::showpos);

        if ((expr.id == 0) || (expr.factor==0.0)) {
            if (show_plus) {
                os << " + ";
                os.setf(std::ios::showpos);
            }
            os << "0";
            return os;
        }

        if (1 == expr.id) {
            Monomial::format_factor(os, expr.factor, show_plus);
        } else {
            Monomial::format_factor_skip_one(os, expr.factor, show_plus, true);

            const bool show_hash = os.flags() & std::ios::showbase;
            if (show_hash) {
                os.unsetf(std::ios::showbase);
                os << "#" << expr.id;
                os.setf(std::ios::showbase);
            } else {
                os << expr.id;
            }
            if (expr.conjugated) {
                os << "*";
            }
        }
        if (show_plus) {
            os.setf(std::ios::showpos);
        }
        return os;
    }

    Monomial::Monomial(const std::string &strExpr) {
        // Size must be in bounds
        if (strExpr.empty() || (strExpr.size() > Monomial::max_strlen)) {
            throw SymbolParseException{strExpr};
        }
        size_t read_to = strExpr.length();
        if (strExpr.ends_with('*')) {
            this->conjugated = true;
            --read_to;
        } else {
            this->conjugated = false;
        }

        try {
            size_t how_much = 0;
            auto read_me = std::stoi(strExpr, &how_much);

            // Make sure whole string is read
            if (how_much < read_to) {
                throw SymbolParseException{strExpr};
            }

            if (read_me < 0) {
                this->factor = -1.0;
                read_me = -read_me;
            } else {
                this->factor = 1.0;
            }
            this->id = static_cast<symbol_name_t>(read_me);
        }
        catch (std::invalid_argument& e) {
            throw SymbolParseException{strExpr, e};
        }
        catch (std::out_of_range& e) {
            throw SymbolParseException{strExpr, e};
        }
    }

    std::string Monomial::as_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
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