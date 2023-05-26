/**
 * transform_symbols.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "transform_symbols.h"

#include "storage_manager.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include "scenarios/derived/derived_matrix_system.h"
#include "scenarios/derived/symbol_table_map.h"

#include "symbolic/symbol_combo.h"
#include "symbolic/symbol_combo_to_basis.h"

#include "eigen/read_eigen_sparse.h"
#include "export/export_symbol_combo.h"

#include <sstream>

namespace Moment::mex::functions {

    namespace {
        SymbolCombo get_input_as_combo(matlab::engine::MATLABEngine matlabEngine,
                                       const SymbolTable& symbols,
                                       const TransformSymbolsParams& input) {

            if (input.input_type == TransformSymbolsParams::InputType::SymbolId) {
                return SymbolCombo{Monomial{input.symbol_id, 1.0, false}};
            }

            if (input.input_type != TransformSymbolsParams::InputType::Basis) {
                throw_error(matlabEngine, errors::bad_param, "Uknown input type.");
            }

            Eigen::SparseVector<double> real_base =
                    (input.inputs.size()>=2) ? read_eigen_sparse_vector(matlabEngine, input.inputs[1])
                                             : Eigen::SparseVector<double>(0);

            Eigen::SparseVector<double> complex_base =
                    (input.inputs.size()>=3) ? read_eigen_sparse_vector(matlabEngine, input.inputs[2])
                                             : Eigen::SparseVector<double>(0);

            return BasisVecToSymbolCombo{symbols}(real_base, complex_base);

        }
    }

    TransformSymbolsParams::TransformSymbolsParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        if (this->inputs.size() >= 3) {
            this->input_type = InputType::Basis;
        } else {
            // Ascertain input type of second argument
            if (inputs[1].getNumberOfElements() == 1) {
                this->input_type = InputType::SymbolId;
                this->symbol_id = read_positive_integer<symbol_name_t>(matlabEngine, "Symbol ID", this->inputs[1], 0);
            } else {
                this->input_type = InputType::Basis;
            }
        }

        // Switch output type
        if (this->flags.contains(u"string")) {
            this->output_type = OutputType::String;
        } else {
            this->output_type = OutputType::Basis;
        }
    }


    void TransformSymbols::extra_input_checks(TransformSymbolsParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    TransformSymbols::TransformSymbols(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"transform_symbols") {
        this->min_inputs = 2;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"string");
        this->flag_names.emplace(u"combo");
        this->flag_names.emplace(u"basis");
        this->mutex_params.add_mutex({u"string", u"combo", u"basis"});
    }

    void TransformSymbols::operator()(IOArgumentRange output, TransformSymbolsParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        assert(matrixSystemPtr); // ^-- should throw if not found

        const auto& matrixSystem = [&]() -> const Derived::DerivedMatrixSystem& {
            const auto & dms_ptr = dynamic_cast<const Derived::DerivedMatrixSystem*>(matrixSystemPtr.get());
            if (dms_ptr) {
                return *dms_ptr;
            }
            std::stringstream errSS;
            errSS << "MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec
                  << " was not a derived matrix system.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }();

        // Get input as a symbol combo, as expressed in base matrix system.
        const auto input_combo = get_input_as_combo(this->matlabEngine, matrixSystem.base_system().Symbols(), input);

        if (verbose) {
            std::stringstream inputSS;
            inputSS << "Input: " << input_combo << "\n";
            print_to_console(this->matlabEngine, inputSS.str());
        }

        // Transform input
        const auto& map = matrixSystem.map();
        const auto output_combo = map(input_combo);

        // Export output in new matrix system
        matlab::data::ArrayFactory factory;
        switch (input.output_type) {
            case TransformSymbolsParams::OutputType::String:
                output[0] = [&]() -> matlab::data::Array {
                    return factory.createScalar(output_combo.as_string());
                }();
                break;
            case TransformSymbolsParams::OutputType::Basis: {
                auto [re_part, im_part] = SymbolComboExporter{this->matlabEngine, matrixSystem.Symbols()}(output_combo);
                if (output.size() >= 1) {
                    output[0] = std::move(re_part);
                }
                if (output.size() >= 2) {
                    output[1] = std::move(im_part);
                } else if (!this->quiet) {
                    const matlab::data::SparseArray<double> im_output = im_part;
                    if (im_output.getNumberOfNonZeroElements() > 0) {
                        print_warning(this->matlabEngine, "Output has imaginary basis parts that have been truncated.");
                    }
                }
            }
                break;
            case TransformSymbolsParams::OutputType::Unknown:
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
        }

    }



}