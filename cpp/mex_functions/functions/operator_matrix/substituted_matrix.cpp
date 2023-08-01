/**
 * substituted_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substituted_matrix.h"
#include "storage_manager.h"

#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "symbolic/rules/moment_rulebook.h"

namespace Moment::mex::functions  {


    void SubstitutedMatrixParams::extra_parse_params() {
        assert(inputs.empty());

        // Get matrix index
        auto& matrix_param = this->find_or_throw(u"matrix");
        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'matrix'", matrix_param, 0);

        auto& rules_param = this->find_or_throw(u"rules");
        this->rules_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'rules'", matrix_param, 0);

    }

    void SubstitutedMatrixParams::extra_parse_inputs() {
        assert(inputs.size() == 3);

        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Matrix index", inputs[1], 0);
        this->rules_index = read_positive_integer<size_t>(matlabEngine, "Rulebook index", inputs[2]);
    }

    bool SubstitutedMatrixParams::any_param_set() const {
        return this->params.contains(u"matrix")
            || this->params.contains(u"rules")
            || OperatorMatrixParams::any_param_set();
    }


    SubstitutedMatrix::SubstitutedMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : OperatorMatrix{matlabEngine, storage} {
        this->param_names.emplace(u"matrix");
        this->param_names.emplace(u"rules");
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    SubstitutedMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &omp) {
        auto& avp = dynamic_cast<SubstitutedMatrixParams&>(omp);
        auto mt_policy = this->settings->get_mt_policy();
        return system.SubstitutedMatrix.create(SubstitutedMatrixIndices::Index(avp.matrix_index, avp.rules_index),
                                               mt_policy);
    }
}