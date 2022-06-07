/**
 * symbol_expression.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_expression.h"
#include <iostream>

namespace NPATK {

    std::ostream &operator<<(std::ostream &os, const SymbolExpression& symb) {
        os << symb.as_string();
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const SymbolPair &pair) {
        os << pair.left_id << " == ";
        if (pair.negated) {
            os << "-";
        }
        os << pair.right_id;
        if (pair.conjugated) {
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
                this->negated = true;
                read_me = -read_me;
            } else {
                this->negated = false;
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