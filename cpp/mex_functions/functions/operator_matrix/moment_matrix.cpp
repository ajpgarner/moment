/**
 * moment_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "moment_matrix.h"

#include "storage_manager.h"

#include "matrix/moment_matrix.h"


#include "utilities/read_as_scalar.h"
#include "utilities/io_parameters.h"

#include <memory>

namespace Moment::mex::functions {

    MomentMatrix::MomentMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");

        this->max_inputs = 2;
    }

    void MomentMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);
    }

    void MomentMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as Settings object + depth
        assert(this->inputs.size() == 2); // should be guaranteed by parent
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);
    }

    bool MomentMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        return level_specified || OperatorMatrixParams::any_param_set();
    }

    std::pair<size_t, const Moment::Matrix &>
    MomentMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        const auto& input = dynamic_cast<const MomentMatrixParams&>(inputOMP);

        return system.create_moment_matrix(input.hierarchy_level);
    }

}