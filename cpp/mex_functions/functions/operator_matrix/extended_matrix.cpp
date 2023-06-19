/**
 * extended_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "extended_matrix.h"
#include "storage_manager.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/read_as_string.h"

namespace Moment::mex::functions {

    void ExtendedMatrixParams::extra_parse_params()  {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get extensions

        auto& ext_param = this->find_or_throw(u"extensions");
        this->read_extension_argument("Parameter 'extensions'", ext_param);
    }

    void ExtendedMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as Settings object + depth
        assert(this->inputs.size() == 3); // should be guaranteed by parent

        // Get depth
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);

        // Get extensions
        this->read_extension_argument("Extensions", inputs[2]);
    }

    [[nodiscard]] bool ExtendedMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool extensions_specified = this->params.contains(u"extensions");
        return level_specified || extensions_specified || OperatorMatrixParams::any_param_set();
    }


    void ExtendedMatrixParams::read_extension_argument(const std::string& paramName,
                                                       const matlab::data::Array& input_array) {
        // Try read as a string...
        std::optional<std::basic_string<char16_t>> asString;
        switch(input_array.getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
                if (input_array.getNumberOfElements() == 1) {
                    asString = read_as_utf16(input_array);
                }
                break;
            case matlab::data::ArrayType::CHAR:
                asString = read_as_utf16(input_array);
                break;
            default:
                break;
        }
        if (asString.has_value()) {
            if (!(asString.value() == u"auto")) {
                std::stringstream errSS;
                errSS << paramName << " must either be an array of symbol IDs, or the string 'auto'.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            } else {
                this->extensions.clear();
                this->extension_type = ExtensionType::Automatic;
            }
            return;
        }

        // Otherwise, read manually specified extensions
        this->extensions = read_positive_integer_array<symbol_name_t>(matlabEngine, paramName, input_array, 0);
        this->extension_type = ExtensionType::Manual;
    }


    ExtendedMatrix::ExtendedMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, extensions] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"extensions");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::Matrix&>
    ExtendedMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp)  {
        // Get extended parameters
        auto& emp = dynamic_cast<ExtendedMatrixParams&>(omp);

        // Get inflation matrix system
        auto * inflationSystemPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system);
        if (nullptr == inflationSystemPtr) {
            throw_error(this->matlabEngine, errors::bad_param, "Matrix system reference was not an inflation scenario");
        }
        auto& inflationSystem = *inflationSystemPtr;

        // Make sure moment matrix exists (or create it, otherwise)
        auto [mm_index, mm_op_matrix] = inflationSystem.create_moment_matrix(emp.hierarchy_level);
        //        const auto* mmPtr = MomentMatrix::as_monomial_moment_matrix(mm_op_matrix);
        //        const auto& moment_matrix = *mmPtr;
        const auto& monoMatrix = dynamic_cast<const MonomialMatrix&>(mm_op_matrix);


        if (emp.extension_type == ExtendedMatrixParams::ExtensionType::Manual) {
            // Sanitize manual symbols
            const size_t symbol_count = inflationSystem.Symbols().size();
            for (auto sym_id: emp.extensions) {
                if (sym_id >= symbol_count) {
                    std::stringstream errSS;
                    errSS << "Symbol with ID \"" << sym_id << "\" was not found in matrix system's symbol table.";
                    throw_error(this->matlabEngine, errors::bad_param, errSS.str());
                }
            }
        } else {
            // Get automatic symbols
            auto lock = inflationSystem.get_read_lock();
            auto extension_set = inflationSystem.suggest_extensions(monoMatrix);
            emp.extensions.clear();
            emp.extensions.reserve(extension_set.size());
            std::copy(extension_set.cbegin(), extension_set.cend(), std::back_inserter(emp.extensions));
        }

        // Verbose output
        if (this->verbose) {
            std::stringstream ss;
            ss << "Extended " << monoMatrix.Description() << " "
                << (emp.extension_type == ExtendedMatrixParams::ExtensionType::Automatic ? "automatically" : "manually")
                << " with " << emp.extensions.size() << (emp.extensions.size()!=1 ? " extensions" : " extension")
                << ": ";
            bool done_one = false;
            for (auto ex : emp.extensions) {
                if (done_one) {
                    ss << ", ";
                } else {
                    done_one = true;
                }
                ss << "S" << ex;
            }
            ss << ".\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Now, call for extension
        return inflationSystem.create_extended_matrix(monoMatrix, emp.extensions);
    }

}