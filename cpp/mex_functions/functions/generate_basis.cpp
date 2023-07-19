/**
 * generate_basis.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "generate_basis.h"

#include "storage_manager.h"

#include "matrix/operator_matrix/operator_matrix.h"

#include "export/export_basis.h"
#include "export/export_matrix_basis_masks.h"
#include "export/export_polynomial.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include <array>
#include <complex>

namespace Moment::mex::functions {

    namespace {
        const Matrix& getMatrixOrThrow(matlab::engine::MATLABEngine &matlabEngine,
                                               const MatrixSystem& matrixSystem, size_t index) {
            try {
                return matrixSystem[index];
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(matlabEngine, errors::bad_param, mce.what());
            }
        }
    }

    GenerateBasisParams::GenerateBasisParams(Moment::mex::SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        if (this->inputs[1].getType() == matlab::data::ArrayType::CELL) {
            this->input_type = InputType::SymbolCell;
            this->read_symbol_cell(this->inputs[1]);
        } else {
            this->input_type = InputType::MatrixId;
            this->input_data.emplace<0>(
                    read_positive_integer<uint64_t>(matlabEngine, "Matrix index", this->inputs[1], 0)
            );
            this->input_shape.clear();
        }

        // Choose basis output type
        if (this->flags.contains(u"cell")) {
            this->monolithic_output = false;
        } else if (this->flags.contains(u"monolith")) {
            this->monolithic_output = true;
        }

        // Choose output matrix sparsity
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }

    void GenerateBasisParams::read_symbol_cell(matlab::data::Array &raw_input) {
        this->input_type = InputType::SymbolCell;
        if (raw_input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial mode expects symbol cell input.");
        }

        const auto input_dims = raw_input.getDimensions();
        this->input_shape.reserve(input_dims.size());
        std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

        this->input_data.emplace<1>();
        auto& rawPolynomials = std::get<1>(input_data);
        rawPolynomials.reserve(raw_input.getNumberOfElements());

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = raw_input;
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            rawPolynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine, "Input", *read_iter));
            ++read_iter;
        }
    }

    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->flag_names.emplace(u"cell");
        this->flag_names.emplace(u"monolith");
        this->mutex_params.add_mutex(u"cell", u"monolith");
    }



    void GenerateBasis::extra_input_checks(GenerateBasisParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }


    void GenerateBasis::operator()(IOArgumentRange output, GenerateBasisParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto lock = matrixSystem.get_read_lock();

        switch (input.input_type) {
            case GenerateBasisParams::InputType::MatrixId:
                this->generate_matrix_basis(output, input, matrixSystem);
                break;
            case GenerateBasisParams::InputType::SymbolCell:
                this->generate_symbol_cell_basis(output, input, matrixSystem);
                break;
        }
    }

    void GenerateBasis::generate_matrix_basis(IOArgumentRange &output, GenerateBasisParams &input,
                                              const MatrixSystem& matrixSystem) {
        const auto& symbolic_matrix = [&]() -> const Matrix& {
            try {
                return matrixSystem[input.matrix_index()];
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(matlabEngine, errors::bad_param, mce.what());
            }
        }();

        const bool complex_output = symbolic_matrix.HasComplexBasis();

        // Complex output requires two outputs... give warning
        if (!this->quiet && complex_output && (output.size() < 2)) {
            print_warning(this->matlabEngine,
                          "Matrix is potentially complex, but the imaginary element output has not been bound."
            );
        }

        // Do generation of basis
        BasisExporter exporter{this->matlabEngine, input.sparse_output, input.monolithic_output};
        auto [sym, anti_sym] = exporter(symbolic_matrix);
        output[0] = std::move(sym);
        if (output.size() >= 2) {
            output[1] = std::move(anti_sym);
        }

        // If enough outputs supplied, also provide basis key
        if (output.size() >= 3) {
            BasisKeyExporter bke{this->matlabEngine};
            output[2] = bke.basis_key(symbolic_matrix);
        }
    }

    void GenerateBasis::generate_symbol_cell_basis(IOArgumentRange &output, GenerateBasisParams &input,
                                                   const MatrixSystem& matrixSystem) {
        const auto& poly_factory = matrixSystem.polynomial_factory();

        // Read in polynomials
        std::vector<Polynomial> polynomial;
        polynomial.reserve(input.raw_polynomials().size());
        for (const auto& raw_poly : input.raw_polynomials()) {
            polynomial.emplace_back(raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly));
        }

        // Export monolithic basis
        matlab::data::ArrayFactory factory;
        PolynomialExporter exporter{matlabEngine, factory, matrixSystem.Symbols(), poly_factory.zero_tolerance};
        std::tie(output[0], output[1]) = exporter.basis(polynomial);
    }
}
