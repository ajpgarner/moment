/**
 * export_moment_substitution_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_moment_substitution_rules.h"

#include "scenarios/context.h"

#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/rules/moment_rulebook_to_basis.h"
#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"


#include "utilities/utf_conversion.h"

#include "eigen/export_eigen_sparse.h"


namespace Moment::mex {

    matlab::data::CellArray MomentSubstitutionRuleExporter::as_symbol_cell(const MomentRulebook &rules) {
        auto output = factory.createCellArray({rules.size(), 1});
        auto write_iter = output.begin();
        for (const auto& rule : rules) {
            auto polynomial = rule.second.as_polynomial(rules.factory);
            *write_iter = this->polynomial_exporter.symbol_cell(polynomial);
            ++write_iter;
        }
        return output;
    }

    matlab::data::CellArray
    MomentSubstitutionRuleExporter::as_polynomials(const Moment::MomentRulebook &rules) {
        auto output = factory.createCellArray({rules.size(), 1});
        auto write_iter = output.begin();
        for (const auto& rule : rules) {
            auto polynomial = rule.second.as_polynomial(rules.factory);
            auto poly_data = this->polynomial_exporter.sequences(polynomial, true);
            *write_iter = poly_data.move_to_cell(factory);
            ++write_iter;
        }
        return output;
    }

    matlab::data::StringArray MomentSubstitutionRuleExporter::as_string(const MomentRulebook &rules) {
        auto output = factory.createArray<matlab::data::MATLABString>({rules.size(), 1});
        auto write_iter = output.begin();
        if (string_format_options.as_operators) {
            for (const auto &rule: rules) {
                *write_iter = this->write_rule_string_as_operator(rule.second);
                ++write_iter;
            }
        } else {
            for (const auto &rule: rules) {
                *write_iter = this->write_rule_string_as_symbol(rule.second);
                ++write_iter;
            }
        }
        return output;
    }


    matlab::data::MATLABString
    MomentSubstitutionRuleExporter::write_rule_string_as_operator(const MomentRule &rule) {
        std::stringstream ruleSS;
        ContextualOS cSS{ruleSS, this->context, this->symbols};
        cSS.format_info.show_braces = string_format_options.show_braces;
        cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

        if (rule.LHS() < this->symbols.size()) {
            const auto& symbolInfo= this->symbols[rule.LHS()];
            if (symbolInfo.has_sequence()) {
                cSS << symbolInfo.sequence();
            } else {
                cSS.context.format_sequence_from_symbol_id(cSS, rule.LHS(), false);
            }
        } else {
            ruleSS << "UNK#" << rule.LHS();
        }

        ruleSS << "  ->  ";

        cSS << rule.RHS();

        return UTF8toUTF16Convertor{}(ruleSS.str());
    }

    matlab::data::MATLABString
    MomentSubstitutionRuleExporter::write_rule_string_as_symbol(const MomentRule &rule) {
        std::stringstream ruleSS;
        ruleSS << "#" << rule.LHS() << "  ->  " << rule.RHS();
        return UTF8toUTF16Convertor{}(ruleSS.str());
    }

    matlab::data::Array MomentSubstitutionRuleExporter::as_rewrite_matrix(const MomentRulebook &rulebook) {
        MomentRulebookToBasis mrtb{this->symbols, this->polynomial_exporter.zero_tolerance,
                                   MomentRulebookToBasis::ExportMode::Rewrite};
        auto eigen_sparse_matrix = mrtb(rulebook);
        return export_eigen_sparse(this->engine, factory, eigen_sparse_matrix);
    }

    matlab::data::Array MomentSubstitutionRuleExporter::as_homogenous_matrix(const Moment::MomentRulebook &rulebook) {
        MomentRulebookToBasis mrtb{this->symbols, this->polynomial_exporter.zero_tolerance,
                                   MomentRulebookToBasis::ExportMode::Homogeneous};
        auto eigen_sparse_matrix = mrtb(rulebook);
        return export_eigen_sparse(this->engine, factory, eigen_sparse_matrix);
    }

}