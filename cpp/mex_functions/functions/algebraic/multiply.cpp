/**
 * multiply.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "multiply.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include "storage_manager.h"

#include "scenarios/context.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"

#include "export/export_operator_matrix.h"
#include "export/export_polynomial.h"

namespace Moment::mex::functions {

    namespace {

        [[nodiscard]] const MonomialMatrix&
        input_to_monomial_matrix(matlab::engine::MATLABEngine& matlabEngine,
                                 const MatrixSystem& matrixSystem, const MultiplyParams::Operand& input) {
            // Get matrix, or throw
            auto matrix_id = input.matrix_key();
            if (matrixSystem.size() <= matrix_id) {
                std::stringstream errSS;
                errSS << "Matrix with ID '" << matrix_id << "' is out of range.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }
            auto& matrix = matrixSystem[input.matrix_key()];

            if (!matrix.is_monomial()) {
                throw_error(matlabEngine, errors::internal_error,
                            "Polynomial matrix multiplication not yet supported");
            }

            if (!matrix.has_operator_matrix()) {
                throw_error(matlabEngine, errors::bad_param,
                            "Cannot multiply matrix that is not explicitly defined by monomial operators.");
            }
            return dynamic_cast<const MonomialMatrix&>(matrix);
        }

        [[nodiscard]] const Polynomial input_to_polynomial(matlab::engine::MATLABEngine& matlabEngine,
                                                           const MatrixSystem& system,
                                                           const MultiplyParams::Operand& input) {
            assert(!input.raw_polynomials().empty());
            auto& first_poly_raw = input.raw_polynomials().front();
            return raw_data_to_polynomial(matlabEngine, system.polynomial_factory(), first_poly_raw);
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        matrix_by_polynomial(matlab::engine::MATLABEngine& matlabEngine,
                             const MultiplyParams &input, MatrixSystem &matrixSystem) {

            // Get read lock on matrix system
            auto write_lock = matrixSystem.get_write_lock();

            const auto& matrix = input_to_monomial_matrix(matlabEngine, matrixSystem, input.lhs);
            const auto& polynomial = input_to_polynomial(matlabEngine, matrixSystem, input.rhs);

            if (polynomial.is_monomial() && (!polynomial.empty())) {
                return matrix.post_multiply(polynomial.back(), matrixSystem.Symbols(),
                                            Multithreading::MultiThreadPolicy::Optional);
            } else {
                return matrix.post_multiply(polynomial, matrixSystem.polynomial_factory(), matrixSystem.Symbols(),
                                            Multithreading::MultiThreadPolicy::Optional);
            }
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        polynomial_by_matrix(matlab::engine::MATLABEngine& matlabEngine,
                             const MultiplyParams &input, MatrixSystem &matrixSystem) {

            // Get read lock on matrix system
            auto write_lock = matrixSystem.get_write_lock();

            const auto& polynomial = input_to_polynomial(matlabEngine, matrixSystem, input.lhs);
            const auto& matrix = input_to_monomial_matrix(matlabEngine, matrixSystem, input.rhs);

            if (polynomial.is_monomial() && (!polynomial.empty())) {
                return matrix.pre_multiply(polynomial.back(), matrixSystem.Symbols(),
                                            Multithreading::MultiThreadPolicy::Optional);
            } else {
                return matrix.pre_multiply(polynomial, matrixSystem.polynomial_factory(), matrixSystem.Symbols(),
                                            Multithreading::MultiThreadPolicy::Optional);
            }
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        do_multiplication(matlab::engine::MATLABEngine& matlabEngine,
                          const MultiplyParams &input, MatrixSystem &matrixSystem) {
            switch (input.lhs.type) {
                case MultiplyParams::Operand::InputType::MatrixID:
                    switch (input.rhs.type) {
                        case MultiplyParams::Operand::InputType::MatrixID:
                            throw_error(matlabEngine, errors::internal_error, "Matrix RHS not yet implemented.");
                        case MultiplyParams::Operand::InputType::Polynomial:
                            return matrix_by_polynomial(matlabEngine, input, matrixSystem);
                        default:
                        case MultiplyParams::Operand::InputType::Unknown:
                            throw_error(matlabEngine, errors::bad_param, "Cannot multiply unknown RHS type.");
                    }
                    break;
                case MultiplyParams::Operand::InputType::Polynomial:
                    switch (input.rhs.type) {
                        case MultiplyParams::Operand::InputType::MatrixID:
                            return polynomial_by_matrix(matlabEngine, input, matrixSystem);
                        case MultiplyParams::Operand::InputType::Polynomial:
                            throw_error(matlabEngine, errors::internal_error, "Polynomial RHS not yet implemented.");
                            break;
                        default:
                        case MultiplyParams::Operand::InputType::Unknown:
                            throw_error(matlabEngine, errors::bad_param, "Cannot multiply unknown RHS type.");
                    }
                    break;
                default:
                case MultiplyParams::Operand::InputType::Unknown:
                    throw_error(matlabEngine, errors::bad_param, "Cannot multiply unknown LHS type.");
            }
        }


        void output_as_monomial(matlab::engine::MATLABEngine& matlabEngine, IOArgumentRange& output,
                                MultiplyParams::OutputMode output_mode,
                                const MatrixSystem& matrixSystem, const MonomialMatrix& matrix) {
            matlab::data::ArrayFactory factory;
            if (output.size() >= 2) {
                output[1] = factory.createScalar<bool>(true);
            }

            OperatorMatrixExporter ome{matlabEngine, matrixSystem};

            switch (output_mode) {
                case MultiplyParams::OutputMode::MatrixIndex:
                    throw_error(matlabEngine, errors::bad_param,
                                "output_as_monomial MatrixIndex output not yet implemented.");
                    break;
                case MultiplyParams::OutputMode::String:
                    output[0] = ome.sequence_strings(matrix);
                    break;
                case MultiplyParams::OutputMode::SymbolCell:
                    output[0] = ome.symbol_cell(matrix);
                    break;
                case MultiplyParams::OutputMode::SequencesWithSymbolInfo: {
                    auto fms = ome.monomials(matrix);
                    output[0] = fms.move_to_cell(factory);
                } break;
                case MultiplyParams::OutputMode::Unknown:
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output format for monomial matrix.");
            }
        }

        void output_as_polynomial(matlab::engine::MATLABEngine& matlabEngine, IOArgumentRange& output,
                                  MultiplyParams::OutputMode output_mode,
                                  const MatrixSystem& matrixSystem, const PolynomialMatrix& matrix) {
            matlab::data::ArrayFactory factory;

            auto read_lock = matrixSystem.get_read_lock();

            if (output.size() >= 2) {
                output[1] = factory.createScalar<bool>(false);
            }

            // Export polynomial matrix
            const auto& poly_factory = matrixSystem.polynomial_factory();
            PolynomialExporter exporter{matlabEngine, factory,
                                        matrixSystem.Context(), matrixSystem.Symbols(), poly_factory.zero_tolerance};
            matlab::data::ArrayDimensions dimensions{matrix.Dimension(), matrix.Dimension()};
            switch (output_mode) {
                case MultiplyParams::OutputMode::MatrixIndex:
                    throw_error(matlabEngine, errors::bad_param,
                                "output_as_polynomial MatrixIndex output not yet implemented.");
                    break;
                case MultiplyParams::OutputMode::String: {
                    matlab::data::StringArray string_out =
                            factory.createArray<matlab::data::MATLABString>(dimensions);

                    std::transform(matrix.SymbolMatrix().begin(), matrix.SymbolMatrix().end(), string_out.begin(),
                                   [&exporter](const Polynomial &poly) -> matlab::data::MATLABString {
                                       return exporter.string(poly);
                                   });
                    output[0] = std::move(string_out);
                } break;
                case MultiplyParams::OutputMode::SymbolCell:
                    output[0] = exporter.symbol_cell_vector(matrix.SymbolMatrix(), dimensions);
                break;
                case MultiplyParams::OutputMode::SequencesWithSymbolInfo:
                    output[0] = exporter.sequence_cell_vector(matrix.SymbolMatrix(), dimensions, true);
                break;
                case MultiplyParams::OutputMode::Unknown:
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output format for polynomial matrix.");
            }
        }
    }

    MultiplyParams::MultiplyParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Check type of LHS input
        switch (this->inputs[1].getType()) {
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
                this->lhs = this->parse_as_matrix_key("LHS", this->inputs[1]);
                break;
            case matlab::data::ArrayType::CELL:
                this->lhs = this->parse_as_polynomial("LHS", this->inputs[1]);
                break;
            default:
            case matlab::data::ArrayType::UNKNOWN:
                throw_error(matlabEngine, errors::bad_param, "LHS was not a valid multiplicand type.");
                break;
        }

        // Check type of RHS input
        switch (this->inputs[2].getType()) {
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
                this->rhs = this->parse_as_matrix_key("RHS", this->inputs[2]);
                break;
            case matlab::data::ArrayType::CELL:
                this->rhs = this->parse_as_polynomial("RHS", this->inputs[2]);
                break;
            default:
            case matlab::data::ArrayType::UNKNOWN:
                throw_error(matlabEngine, errors::bad_param, "RHS was not a valid multiplicand type.");
        }

        // How do we output?
        if (this->flags.contains(u"strings")) {
            this->output_mode = OutputMode::String;
        } else if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::SequencesWithSymbolInfo;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::SymbolCell;
        } else {
            this->output_mode = OutputMode::MatrixIndex;
        }
    }

    MultiplyParams::Operand MultiplyParams::parse_as_matrix_key(const std::string& name,
                                                                matlab::data::Array& raw_input) {
        if (raw_input.getNumberOfElements() != 1) {
            throw_error(this->matlabEngine, errors::bad_param, "Matrix index input must be a scalar integer.");
        }

        // Read key
        Operand raw;
        raw.type = Operand::InputType::MatrixID;
        raw.shape = std::vector<size_t>{0, 0};
        raw.raw.emplace<0>(read_as_uint64(this->matlabEngine, raw_input));
        return raw;
    }

    MultiplyParams::Operand MultiplyParams::parse_as_polynomial(const std::string& name,
                                                                matlab::data::Array& raw_input) {
        const size_t expected_elements = raw_input.getNumberOfElements();
        if (expected_elements == 0) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial multiplication expects non-empty operand.");
        }
        Operand raw;
        raw.type = expected_elements!= 1 ? Operand::InputType::PolynomialArray
                                                        : Operand::InputType::Polynomial;
        const auto dimensions = raw_input.getDimensions();
        raw.shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(raw.shape));

        if (raw_input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial mode expects symbol cell input.");
        }

        raw.raw.emplace<1>();
        auto& raw_vec = raw.raw_polynomials();
        raw_vec.reserve(expected_elements);

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = raw_input;
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            raw_vec.emplace_back(read_raw_polynomial_data(this->matlabEngine, name, *read_iter));
            ++read_iter;
        }
        return raw;
    }

    void Multiply::extra_input_checks(MultiplyParams& input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Multiply::Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 2;
        this->max_outputs = 4;

        this->flag_names.emplace(u"index");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"index", u"symbols", u"sequences", u"strings"});
    }

    void Multiply::operator()(IOArgumentRange output, MultiplyParams &input) {
        // Match outputs with mode
        const bool save_index_mode = (input.output_mode == MultiplyParams::OutputMode::MatrixIndex);
        if (save_index_mode) {
            if (output.size() < 4) {
                throw_error(matlabEngine, errors::too_few_outputs, "Four outputs are required.");
            }
        } else {
            if (output.size() > 2) {
                throw_error(matlabEngine, errors::too_many_outputs, "Only 2 outputs expected.");
            }
        }

        // Get handle to matrix system
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        assert(matrixSystemPtr); // ^-- should throw if not found
        MatrixSystem& matrixSystem = *matrixSystemPtr;

        // Do multiplication
        auto multiplied_matrix_ptr = do_multiplication(this->matlabEngine, input, matrixSystem);
        const bool output_is_monomial = multiplied_matrix_ptr->is_monomial();

        // Save matrix if requested
        if (input.output_mode == MultiplyParams::OutputMode::MatrixIndex) {
            const size_t output_dimension = multiplied_matrix_ptr->Dimension();
            const bool is_hermitian = multiplied_matrix_ptr->Hermitian();
            auto write_lock = matrixSystem.get_write_lock();
            const ptrdiff_t matrix_index = matrixSystem.push_back(write_lock, std::move(multiplied_matrix_ptr));
            write_lock.unlock();

            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar<int64_t>(matrix_index);
            output[1] = factory.createScalar<size_t>(output_dimension);
            output[2] = factory.createScalar<bool>(output_is_monomial);
            output[3] = factory.createScalar<bool>(is_hermitian);
            return;
        }

        // Otherwise, export polynomial data
        if (output_is_monomial) {
            output_as_monomial(this->matlabEngine, output, input.output_mode,
                               matrixSystem, dynamic_cast<const MonomialMatrix&>(*multiplied_matrix_ptr));
        } else {
            output_as_polynomial(this->matlabEngine, output, input.output_mode,
                                 matrixSystem, dynamic_cast<const PolynomialMatrix&>(*multiplied_matrix_ptr));
        }
    }
}
