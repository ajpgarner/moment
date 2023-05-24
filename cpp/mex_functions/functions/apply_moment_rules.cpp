/**
 * apply_values.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "apply_moment_rules.h"
#include "storage_manager.h"

#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "symbolic/moment_substitution_rulebook.h"

namespace Moment::mex::functions  {


    void ApplyMomentRulesParams::extra_parse_params() {
        assert(inputs.empty());

        // Get matrix index
        auto& matrix_param = this->find_or_throw(u"matrix");
        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'matrix'", matrix_param, 0);

        auto& rules_param = this->find_or_throw(u"rules");
        this->rules_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'rules'", matrix_param, 0);

    }

    void ApplyMomentRulesParams::extra_parse_inputs() {
        assert(inputs.size() == 3);

        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Matrix index", inputs[1], 0);
        this->rules_index = read_positive_integer<size_t>(matlabEngine, "Rulebook index", inputs[2]);
    }

    bool ApplyMomentRulesParams::any_param_set() const {
        return this->params.contains(u"matrix")
            || this->params.contains(u"rules")
            || OperatorMatrixParams::any_param_set();
    }


    ApplyMomentRules::ApplyMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : OperatorMatrix{matlabEngine, storage, u"apply_moment_rules"} {
        this->param_names.emplace(u"matrix");
        this->param_names.emplace(u"rules");
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::Matrix &>
    ApplyMomentRules::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &omp) {
        auto& avp = dynamic_cast<ApplyMomentRulesParams&>(omp);
        return system.clone_and_substitute(avp.matrix_index, avp.rules_index);
    }
}