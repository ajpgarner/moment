/**
 * symbol_tools.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol_tools.h"

#include "symbol_combo.h"
#include "monomial.h"
#include "symbol_table.h"

namespace Moment {
    void SymbolTools::make_canonical(Monomial& expr) const {
        assert(expr.id >= 0 && expr.id < this->table.size());
        const auto& symbolInfo = this->table[expr.id];

        if (expr.id == 0) {
            expr.factor = 0.0;
            expr.conjugated = false;
        }

        if (symbolInfo.is_hermitian()) {
            expr.conjugated = false;
        }
        if (symbolInfo.is_antihermitian() && expr.conjugated) {
            expr.factor *= -1;
            expr.conjugated = false;
        }
    }

    void SymbolTools::make_canonical(SymbolCombo& combo) const {
        combo.fix_cc_in_place(this->table, true);
    }

}