/**
 * moment_substitution_rulebook.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rulebook.h"

#include "symbol_table.h"

namespace Moment {

    MomentSubstitutionRulebook::MomentSubstitutionRulebook(const SymbolTable &symbolTable)
        : symbols{symbolTable} {

    }
}