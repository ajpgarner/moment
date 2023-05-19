/**
 * symbol_tools.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once


namespace Moment {
    class SymbolTable;
    class SymbolExpression;
    class SymbolCombo;

    class SymbolTools {
    public:
        const SymbolTable& table;

        SymbolTools(const SymbolTable& table) : table{table} { }

        void make_canonical(SymbolExpression& expr) const;

        void make_canonical(SymbolCombo& combo) const;
    };


}