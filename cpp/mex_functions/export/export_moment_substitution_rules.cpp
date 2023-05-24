/**
 * export_moment_substitution_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_moment_substitution_rules.h"

#include "symbolic/moment_substitution_rule.h"
#include "symbolic/moment_substitution_rulebook.h"


namespace Moment::mex {

    matlab::data::CellArray MomentSubstitutionRuleExporter::operator()(const MomentSubstitutionRulebook &rules) {
        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({rules.size(), 1});
        auto write_iter = output.begin();
        for (const auto& rule : rules) {
            *write_iter = this->write_rule(factory, rule.second);
            ++write_iter;
        }
        return output;
    }

    matlab::data::CellArray MomentSubstitutionRuleExporter::write_rule(matlab::data::ArrayFactory &factory,
                                                                       const MomentSubstitutionRule &rule) {
        auto output = factory.createCellArray({1, 2});

        output[0] = factory.createScalar(static_cast<uint64_t>(rule.LHS()));
        output[1] = this->combo_exporter.direct(rule.RHS());

        return output;
    }

}