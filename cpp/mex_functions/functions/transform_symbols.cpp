/**
 * transform_symbols.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "transform_symbols.h"

#include "storage_manager.h"

#include "utilities/reporting.h"
#include "utilities/read_choice.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include "scenarios/derived/derived_matrix_system.h"
#include "scenarios/derived/symbol_table_map.h"

#include "symbolic/symbol_table.h"
#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/polynomial_to_basis.h"

#include "eigen/read_eigen_sparse.h"
#include "export/export_polynomial.h"

#include <sstream>

namespace Moment::mex::functions {

//        Polynomial get_input_as_combo(matlab::engine::MATLABEngine matlabEngine,
//                                      const PolynomialFactory& factory,
//                                      const TransformSymbolsParams& input) {
//
//            if (input.input_type == TransformSymbolsParams::InputType::SymbolId) {
//                return Polynomial{Monomial{input.symbol_id, 1.0, false}};
//            }
//
//            if (input.input_type != TransformSymbolsParams::InputType::Basis) {
//                throw_error(matlabEngine, errors::bad_param, "Unknown input type.");
//            }
//
//            Eigen::SparseVector<double> real_base =
//                    (input.inputs.size()>=2) ? read_eigen_sparse_vector(matlabEngine, input.inputs[1])
//                                             : Eigen::SparseVector<double>(0);
//
//            Eigen::SparseVector<double> complex_base =
//                    (input.inputs.size()>=3) ? read_eigen_sparse_vector(matlabEngine, input.inputs[2])
//                                             : Eigen::SparseVector<double>(0);
//
//            return BasisVecToPolynomial{factory}(real_base, complex_base);
//
//        }

    namespace {
        void output_from_polynomials( matlab::engine::MATLABEngine& matlabEngine,
                                      IOArgumentRange& output, TransformSymbolsParams& input,
                                      const Derived::DerivedMatrixSystem& targetSystem,
                                      std::vector<Polynomial>& polys) {

        // Export output in new matrix system
        matlab::data::ArrayFactory factory;
        PolynomialExporter exporter{matlabEngine, factory, targetSystem.Context(), targetSystem.Symbols(),
                                    targetSystem.polynomial_factory().zero_tolerance};

        switch (input.output_type) {
            case TransformSymbolsParams::OutputType::String:
                output[0] = [&]() -> matlab::data::StringArray {
                    auto output = factory.createArray<matlab::data::MATLABString>(input.input_shape);
                    std::transform(polys.cbegin(), polys.cend(), output.begin(), [&](const Polynomial& poly){
                        return exporter.string(poly, true);
                    });
                    return output;
                }();
                break;
            case TransformSymbolsParams::OutputType::Basis:
                std::tie(output[0], output[1]) = exporter.basis(polys);
                break;
            case TransformSymbolsParams::OutputType::SymbolCell:
                output[0] = [&]() -> matlab::data::CellArray {
                    auto output = factory.createCellArray(input.input_shape);
                    std::transform(polys.cbegin(), polys.cend(), output.begin(), [&](const Polynomial& poly){
                        return exporter.symbol_cell(poly);
                    });
                    return output;
                }();
                break;
            case TransformSymbolsParams::OutputType::Unknown:
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output type.");

            }
        }

        inline std::vector<Polynomial>
        transform_polynomials(const Derived::DerivedMatrixSystem& targetSystem,
                              const std::vector<Polynomial>& input_poly) {
            std::vector<Polynomial> output_poly;
            output_poly.reserve(input_poly.size());

            const auto &map = targetSystem.map();
            std::transform(input_poly.cbegin(), input_poly.cend(), std::back_inserter(output_poly),
                           [&map](const auto &poly) { return map(poly); });

            return output_poly;
        }
    }

    TransformSymbolsParams::TransformSymbolsParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Infer input type
        if (this->inputs.size() >= 3) {
            this->input_type = InputType::Basis;
        } else {
            switch (this->inputs[1].getType()) {
                case matlab::data::ArrayType::MATLAB_STRING:
                case matlab::data::ArrayType::DOUBLE:
                case matlab::data::ArrayType::SINGLE:
                case matlab::data::ArrayType::INT8:
                case matlab::data::ArrayType::UINT8:
                case matlab::data::ArrayType::INT16:
                case matlab::data::ArrayType::UINT16:
                case matlab::data::ArrayType::INT32:
                case matlab::data::ArrayType::UINT32:
                case matlab::data::ArrayType::INT64:
                case matlab::data::ArrayType::UINT64:
                    this->input_type = InputType::SymbolId;
                    break;
                case matlab::data::ArrayType::CELL:
                    this->input_type = InputType::SymbolCell;
                    break;
                default:
                    throw_error(matlabEngine, errors::bad_param, "Expected list of symbol IDs, or a symbol cell as input.");
            }
        }

        // Read parameters
        switch (this->input_type) {
            case InputType::SymbolId:
                this->read_symbol_ids(this->inputs[1]);
                break;
            case InputType::SymbolCell:
                this->read_symbol_cell(this->inputs[1]);
                break;
            case InputType::Basis:
                this->read_basis(this->inputs[1], this->inputs[2]);
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown input type.");
        }

        // Determine output choice
        if (auto outParamIter = this->params.find(u"output"); outParamIter != this->params.cend()) {
            switch (read_choice("Parameter 'output'", {"string", "symbols", "basis"}, outParamIter->second)) {
                case 0:
                    this->output_type = OutputType::String;
                    break;
                case 1:
                    this->output_type = OutputType::SymbolCell;
                    break;
                case 2:
                    this->output_type = OutputType::Basis;
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
            }
        } else {
            // Default output type based off input choice.
            switch (this->input_type) {
                case InputType::SymbolId:
                case InputType::SymbolCell:
                    this->output_type = OutputType::SymbolCell;
                    break;
                case InputType::Basis:
                    this->output_type = OutputType::Basis;
                    break;
                case InputType::Unknown:
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown input type.");
                    break;
            }
        }

    }


    void TransformSymbolsParams::read_symbol_ids(matlab::data::Array& raw_input) {
        // Determine input dimensions
        const auto& input_dims = raw_input.getDimensions();
        this->input_shape.reserve(input_dims.size());
        std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

        // Try to read data from array
        std::get<0>(this->input_data) = read_integer_array<symbol_name_t>(this->matlabEngine,
                                                                          "Symbol ID list", raw_input);
    }

    void TransformSymbolsParams::read_symbol_cell(matlab::data::Array& raw_input) {
        const auto& input_dims = raw_input.getDimensions();
        this->input_shape.reserve(input_dims.size());
        std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

        this->input_data.emplace<1>();
        auto& rawPolynomials = std::get<1>(this->input_data);
        const matlab::data::CellArray cell_input = raw_input;
        rawPolynomials.reserve(cell_input.getNumberOfElements());

        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            rawPolynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine, "Symbol Cell", *read_iter));
            ++read_iter;
        }
    }

    void TransformSymbolsParams::read_basis(matlab::data::Array& raw_real, matlab::data::Array& raw_imaginary) {
        const auto& re_dims = raw_real.getDimensions();

        throw_error(matlabEngine, errors::bad_param, "TransformSymbolsParams::read_basis not implemented.");
    }


    void TransformSymbols::extra_input_checks(TransformSymbolsParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    TransformSymbols::TransformSymbols(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->param_names.emplace(u"output");
    }

    void TransformSymbols::operator()(IOArgumentRange output, TransformSymbolsParams &input) {
        // Check output count.
        if (input.output_type == TransformSymbolsParams::OutputType::Basis) {
            if (output.size() != 2) {
                throw_error(matlabEngine, errors::too_few_outputs, "Basis export requires two outputs (real & imaginary).");
            }
        } else {
            if (output.size() != 1) {
                throw_error(matlabEngine, errors::too_many_outputs, "Only basis export requires two outputs.");
            }
        }

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

        // Get read locks on source and target systems
        auto target_lock = matrixSystem.get_read_lock();
        auto source_lock = matrixSystem.base_system().get_read_lock();

        switch (input.input_type) {
            case TransformSymbolsParams::InputType::SymbolId:
                this->transform_symbol_ids(output, input, matrixSystem);
                break;
            case TransformSymbolsParams::InputType::SymbolCell:
                this->transform_symbol_cells(output, input, matrixSystem);
                break;
            case TransformSymbolsParams::InputType::Basis:
                this->transform_basis(output, input, matrixSystem);
                break;
            case TransformSymbolsParams::InputType::Unknown:
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown input type.");
        }
    }


    void TransformSymbols::transform_symbol_ids(IOArgumentRange &output, TransformSymbolsParams &input,
                                                const Derived::DerivedMatrixSystem &targetSystem) {

        const auto& sourceSystem  = targetSystem.base_system();

        const auto& sourceSymbols = sourceSystem.Symbols();
        // Make trivial polynomials for each of inputs
        std::vector<Polynomial> input_poly;
        input_poly.reserve(input.symbol_id().size());
        for (auto id : input.symbol_id()) {
            if ((id < 0) || (id >= sourceSymbols.size())) {
                std::stringstream errSS;
                errSS << "Symbol " << id << " not defined in source matrix system.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
            input_poly.emplace_back(Monomial{id, 1.0});
        }

        // Map into new matrix system
        std::vector<Polynomial> output_poly{transform_polynomials(targetSystem, input_poly)};

        // Do output
        output_from_polynomials(matlabEngine, output, input, targetSystem, output_poly);

    }

    void TransformSymbols::transform_symbol_cells(IOArgumentRange &output, TransformSymbolsParams &input,
                                                  const Derived::DerivedMatrixSystem &targetSystem) {

        // Make trivial polynomials for each of inputs
        const auto& input_poly_factory = targetSystem.base_system().polynomial_factory();
        std::vector<Polynomial> input_poly;
        input_poly.reserve(input.raw_polynomials().size());
        for (auto& raw_poly : input.raw_polynomials()) {
            input_poly.emplace_back(raw_data_to_polynomial(this->matlabEngine, input_poly_factory, raw_poly));
        }

        // Map into new matrix system
        std::vector<Polynomial> output_poly{transform_polynomials(targetSystem, input_poly)};

        // Do output
        output_from_polynomials(matlabEngine, output, input, targetSystem, output_poly);
    }

    void TransformSymbols::transform_basis(IOArgumentRange &output, TransformSymbolsParams &input,
                                           const Derived::DerivedMatrixSystem &targetSystem) {
        throw std::runtime_error{"TransformSymbols::transform_basis not implemented."};
    }


}