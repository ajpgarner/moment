/**
 * symbol_expression.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_expression.h"

#include <iostream>
#include <sstream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, const SymbolExpression& expr) {
        if (expr.factor == -1.0) {
            os << "-";
        } else if (expr.factor != 1.0) {
            os << expr.factor;
        }
        os << expr.id;
        if (expr.conjugated) {
            os << "*";
        }
        return os;
    }

    SymbolExpression::SymbolExpression(const std::string &strExpr) {
        // Size must be in bounds
        if (strExpr.empty() || (strExpr.size() > SymbolExpression::max_strlen)) {
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

    std::string SymbolExpression::as_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


    std::string SymbolExpression::SymbolParseException::make_msg(const std::string &badExpr) {
        if (badExpr.length() > SymbolExpression::max_strlen) {
            return std::string("Could not parse \"" + badExpr.substr(0, SymbolExpression::max_strlen) + "...\" as a symbol.");
        }
        return std::string("Could not parse \"" + badExpr + "\" as a symbol.");
    }

    std::string SymbolExpression::SymbolParseException::make_msg(const std::string &badExpr, const std::exception &e) {
        if (badExpr.length() > SymbolExpression::max_strlen) {
            return std::string("Could not parse \"" + badExpr.substr(0, SymbolExpression::max_strlen) + "...\" as a symbol."
                               + "\nThe following exception occurred: " + e.what());
        }
        return std::string("Could not parse \"" + badExpr + "\" as a symbol.\nThe following exception occurred: "
                           + e.what());
    }
}