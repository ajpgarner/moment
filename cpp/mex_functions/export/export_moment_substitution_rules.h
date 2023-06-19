/**
 * export_moment_substitution_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"
#include "export_polynomial.h"

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
    struct RuleStringFormatOptions {
    public:
        bool as_operators = true;
        bool show_braces = true;

        RuleStringFormatOptions() = default;
    };


    class MomentSubstitutionRuleExporter : public Exporter {
    public:

    private:
        PolynomialExporter combo_exporter;
        const SymbolTable& symbols;
        RuleStringFormatOptions string_format_options;

    public:
        explicit MomentSubstitutionRuleExporter(matlab::engine::MATLABEngine &engine, const SymbolTable& symbols,
                                                const double zero_tolerance,
                                                RuleStringFormatOptions rsfo = RuleStringFormatOptions{}) noexcept
                : Exporter{engine}, combo_exporter{engine, symbols, zero_tolerance},
                  symbols{symbols}, string_format_options{rsfo} { }

        matlab::data::CellArray operator()(const MomentSubstitutionRulebook &rules) {
            return this->as_symbol_cell(rules);
        }

        matlab::data::CellArray as_operator_cell(const MomentSubstitutionRulebook& rules);

        matlab::data::CellArray as_symbol_cell(const MomentSubstitutionRulebook& rules);

        matlab::data::StringArray as_string(const MomentSubstitutionRulebook& rules);

    private:
        matlab::data::CellArray write_rule(matlab::data::ArrayFactory& factory,
                                           const MomentSubstitutionRule& rule);

        matlab::data::MATLABString write_rule_string_as_operator(matlab::data::ArrayFactory& factory,
                                                                 const MomentSubstitutionRule& rule);

        matlab::data::MATLABString write_rule_string_as_symbol(matlab::data::ArrayFactory& factory,
                                                               const MomentSubstitutionRule& rule);





    };
}