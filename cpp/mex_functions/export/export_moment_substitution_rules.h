/**
 * export_moment_substitution_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"
#include "export_symbol_combo.h"

#include "MatlabDataArray.hpp"

#include "integer_types.h"

#include <map>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class SymbolTable;
    class MomentSubstitutionRulebook;
    class MomentSubstitutionRule;
}

namespace Moment::mex {
    class MomentSubstitutionRuleExporter : public Exporter {
    private:
        SymbolComboExporter combo_exporter;

    public:
        explicit MomentSubstitutionRuleExporter(matlab::engine::MATLABEngine &engine, const SymbolTable& symbols) noexcept
                : Exporter(engine), combo_exporter{engine, symbols} {}

        matlab::data::CellArray operator()(const MomentSubstitutionRulebook &rules);

    private:
        matlab::data::CellArray write_rule(matlab::data::ArrayFactory& factory, const MomentSubstitutionRule& rule);

    };
}