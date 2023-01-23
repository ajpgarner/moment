/**
 * extend_matrix.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "extended_matrix.h"
#include "storage_manager.h"

#include "matrix/moment_matrix.h"
#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    void ExtendedMatrixParams::extra_parse_params(matlab::engine::MATLABEngine& matlabEngine)  {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get extensions
        auto& ext_param = this->find_or_throw(u"extensions");
        this->extensions = read_positive_integer_array<symbol_name_t>(matlabEngine, "Parameter 'extensions'",
                                                                      ext_param, 0);
    }

    void ExtendedMatrixParams::extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) {
        // No named parameters... try to interpret inputs as Settings object + depth
        assert(this->inputs.size() == 3); // should be guaranteed by parent

        // Get depth
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);

        // Get extensions
        this->extensions = read_positive_integer_array<symbol_name_t>(matlabEngine, "Extensions", inputs[2], 0);
    }

    [[nodiscard]] bool ExtendedMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool extensions_specified = this->params.contains(u"extensions");
        return level_specified || extensions_specified || OperatorMatrixParams::any_param_set();
    }


    ExtendedMatrix::ExtendedMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : OperatorMatrix{matlabEngine, storage, u"extended_matrix"} {
        // Either [ref, level, extensions] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"extensions");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::SymbolicMatrix&>
    ExtendedMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp)  {
        // Get extended parameters
        const auto& emp = dynamic_cast<const ExtendedMatrixParams&>(omp);

        // Get inflation matrix system
        auto * inflationSystemPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system);
        if (nullptr == inflationSystemPtr) {
            throw_error(this->matlabEngine, errors::bad_param, "Matrix system reference was not an inflation scenario");
        }
        auto& inflationSystem = *inflationSystemPtr;

        // Make sure moment matrix exists (or create it, otherwise)
        auto [mm_index, mm_op_matrix] = inflationSystem.create_moment_matrix(emp.hierarchy_level);
        const auto& moment_matrix = dynamic_cast<const MomentMatrix&>(mm_op_matrix);

        // Sanitize symbols
        const size_t symbol_count = inflationSystem.Symbols().size();
        for (auto sym_id : emp.extensions) {
            if (sym_id >= symbol_count) {
                std::stringstream errSS;
                errSS << "Symbol with ID \"" << sym_id << "\" was not found in matrix system's symbol table.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
        }

        // Now, call for extension
        return inflationSystem.create_extended_matrix(moment_matrix, emp.extensions);
    }

}