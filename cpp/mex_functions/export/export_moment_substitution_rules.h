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
    class Context;
    class SymbolTable;
    class MomentRulebook;
    class MomentRule;
}


namespace Moment::mex {
    struct RuleStringFormatOptions {
    public:
        bool as_operators = true;
        bool show_braces = true;

        RuleStringFormatOptions() = default;
    };


    class MomentSubstitutionRuleExporter : public ExporterWithFactory {
    private:
        const Context& context;
        const SymbolTable& symbols;
        const double zero_tolerance;
        PolynomialExporter polynomial_exporter;
        RuleStringFormatOptions string_format_options;

    public:
        explicit MomentSubstitutionRuleExporter(matlab::engine::MATLABEngine &engine,
                                                const Context& context,
                                                const SymbolTable& symbols,
                                                const double zero_tolerance,
                                                RuleStringFormatOptions rsfo = RuleStringFormatOptions{}) noexcept
                : ExporterWithFactory{engine}, context{context}, symbols{symbols}, zero_tolerance{zero_tolerance},
                  polynomial_exporter{engine, factory, context, symbols, zero_tolerance},
                  string_format_options{rsfo} { }

        matlab::data::CellArray operator()(const MomentRulebook &rules) {
            return this->as_symbol_cell(rules);
        }

        matlab::data::CellArray as_polynomials(const MomentRulebook& rules);

        matlab::data::CellArray as_symbol_cell(const MomentRulebook& rules);

        matlab::data::StringArray as_string(const MomentRulebook& rules);

        matlab::data::Array as_rewrite_matrix(const MomentRulebook &rules);

        matlab::data::Array as_homogenous_matrix(const MomentRulebook &rules);


    private:
        matlab::data::MATLABString write_rule_string_as_operator(const MomentRule& rule);

        matlab::data::MATLABString write_rule_string_as_symbol(const MomentRule& rule);


    };
}