/**
 * moment_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "moment_matrix.h"

#include "storage_manager.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "scenarios/pauli/pauli_matrix_system.h"


#include "utilities/read_as_scalar.h"
#include "utilities/io_parameters.h"

#include <memory>

namespace Moment::mex::functions {

    MomentMatrix::MomentMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"neighbours");
        this->param_names.emplace(u"wrap");

        this->max_inputs = 2;
    }

    void MomentMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get NN if any
        this->parse_optional_params();
    }

    void MomentMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as Settings object + depth
        assert(this->inputs.size() == 2); // should be guaranteed by parent
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);

        this->parse_optional_params();
    }

    void MomentMatrixParams::parse_optional_params() {
        // Get NN if any
        this->find_and_parse(u"neighbours", [this](const matlab::data::Array& nn_param) {
            this->extra_data.nearest_neighbours =
                    read_positive_integer<size_t>(matlabEngine, "Parameter 'neighbours'", nn_param, 0);
        });
        // Get wrap status, if any
        if (this->extra_data.nearest_neighbours > 0) {
            this->find_and_parse(u"wrap", [this](const matlab::data::Array& wrap_param) {
                this->extra_data.wrap =
                        read_as_boolean(matlabEngine, wrap_param);
            });
        }
    }

    bool MomentMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        return level_specified || OperatorMatrixParams::any_param_set();
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    MomentMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        const auto& input = dynamic_cast<const MomentMatrixParams&>(inputOMP);
        const auto mt_policy = this->settings->get_mt_policy();

        if (input.extra_data.nearest_neighbours > 0) {
            auto* pms_ptr = dynamic_cast<Pauli::PauliMatrixSystem*>(&system);
            if (pms_ptr == nullptr) {
                throw_error(matlabEngine, errors::bad_param, "Nearest neighbours can only be set in Pauli scenario.");
            }
            return pms_ptr->PauliMomentMatrices.create(Pauli::NearestNeighbourIndex{input.hierarchy_level,
                                                                                    input.extra_data.nearest_neighbours,
                                                                                    input.extra_data.wrap},
                                                       mt_policy);
        } else {
            return system.MomentMatrix.create(input.hierarchy_level, mt_policy);
        }
    }

}