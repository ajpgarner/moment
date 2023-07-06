/**
 * export_moment_substitution_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_moment_substitution_rules.h"

#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/rules/moment_rulebook_to_basis.h"
#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include "utilities/utf_conversion.h"

#include "eigen/export_eigen_sparse.h"


namespace Moment::mex {

    matlab::data::CellArray MomentSubstitutionRuleExporter::as_symbol_cell(const MomentRulebook &rules) {
        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({rules.size(), 1});
        auto write_iter = output.begin();
        for (const auto& rule : rules) {
            *write_iter = this->write_rule(factory, rule.second);
            ++write_iter;
        }
        return output;
    }

    matlab::data::CellArray
    MomentSubstitutionRuleExporter::as_polynomials(const Moment::MomentRulebook &rules,
                                                   const bool include_symbol_info) {
        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({rules.size(), 1});
        auto write_iter = output.begin();
        for (const auto& rule : rules) {
            auto polynomial = rule.second.as_polynomial(rules.factory);
            auto poly_data = this->combo_exporter.sequences(factory, polynomial, include_symbol_info);
            *write_iter = poly_data.move_to_cell(factory);
            ++write_iter;
        }
        return output;
    }

    matlab::data::StringArray MomentSubstitutionRuleExporter::as_string(const MomentRulebook &rules) {
        matlab::data::ArrayFactory factory;
        auto output = factory.createArray<matlab::data::MATLABString>({rules.size(), 1});
        auto write_iter = output.begin();
        if (string_format_options.as_operators) {
            for (const auto &rule: rules) {
                *write_iter = this->write_rule_string_as_operator(factory, rule.second);
                ++write_iter;
            }
        } else {
            for (const auto &rule: rules) {
                *write_iter = this->write_rule_string_as_symbol(factory, rule.second);
                ++write_iter;
            }
        }
        return output;
    }


    matlab::data::CellArray MomentSubstitutionRuleExporter::write_rule(matlab::data::ArrayFactory &factory,
                                                                       const MomentRule &rule) {
        auto output = factory.createCellArray({1, 2});

        output[0] = factory.createScalar(static_cast<uint64_t>(rule.LHS()));
        output[1] = this->combo_exporter.symbol_cell(rule.RHS());

        return output;
    }

    matlab::data::MATLABString
    MomentSubstitutionRuleExporter::write_rule_string_as_operator(matlab::data::ArrayFactory &factory,
                                                                  const MomentRule &rule) {
        std::stringstream ruleSS;
        if (rule.LHS() < this->symbols.size()) {
            const auto& symbolInfo= this->symbols[rule.LHS()];
            if (symbolInfo.has_sequence()) {
                if (this->string_format_options.show_braces) {
                    ruleSS << "<" << symbolInfo.formatted_sequence() << ">";
                } else {
                    ruleSS << symbolInfo.formatted_sequence();
                }
            } else {
                ruleSS << "S#" << rule.LHS();
            }
        } else {
            ruleSS << "UNK#" << rule.LHS();
        }

        ruleSS << "  ->  ";
        rule.RHS().as_string_with_operators(ruleSS, this->symbols, this->string_format_options.show_braces);

        return UTF8toUTF16Convertor{}(ruleSS.str());
    }

    matlab::data::MATLABString
    MomentSubstitutionRuleExporter::write_rule_string_as_symbol(matlab::data::ArrayFactory &factory,
                                                                const MomentRule &rule) {
        std::stringstream ruleSS;
        ruleSS << "#" << rule.LHS() << "  ->  " << rule.RHS();
        return UTF8toUTF16Convertor{}(ruleSS.str());
    }

    matlab::data::Array MomentSubstitutionRuleExporter::as_rewrite_matrix(const MomentRulebook &rulebook) {
        MomentRulebookToBasis mrtb{this->symbols, this->combo_exporter.zero_tolerance,
                                   MomentRulebookToBasis::ExportMode::Rewrite};
        auto eigen_sparse_matrix = mrtb(rulebook);
        matlab::data::ArrayFactory factory;
        return export_eigen_sparse(this->engine, factory, eigen_sparse_matrix);
    }

    matlab::data::Array MomentSubstitutionRuleExporter::as_homogenous_matrix(const Moment::MomentRulebook &rulebook) {
        MomentRulebookToBasis mrtb{this->symbols, this->combo_exporter.zero_tolerance,
                                   MomentRulebookToBasis::ExportMode::Homogeneous};
        auto eigen_sparse_matrix = mrtb(rulebook);
        matlab::data::ArrayFactory factory;
        return export_eigen_sparse(this->engine, factory, eigen_sparse_matrix);
    }

}