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
    public:

    private:
        const SymbolTable& symbols;

        std::map<symbol_name_t, MomentSubstitutionRule> rules;

        std::vector<SymbolCombo> raw_rules;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table);

        void add_raw_rules(std::vector<SymbolCombo>&& raw);

        [[nodiscard]] SymbolCombo reduce(const SymbolCombo& lhs) {
            return reduce(SymbolCombo{lhs});
        }

        [[nodiscard]] SymbolCombo reduce(SymbolCombo&& lhs);

        bool complete();

    };
}