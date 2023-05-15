/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_substitution_rule.h"

namespace Moment {
    class SymbolTable;

    class MomentSubstitutionRulebook {
    private:
        const SymbolTable& symbols;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table);

    };
}