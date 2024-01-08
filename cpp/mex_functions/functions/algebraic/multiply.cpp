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

#include "matrix/symbolic_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"



#include "export/export_operator_matrix.h"
#include "export/export_polynomial.h"

namespace Moment::mex::functions {

    namespace {

        [[nodiscard]] const MonomialMatrix&
        input_to_monomial_matrix(matlab::engine::MATLABEngine& matlabEngine,
                                 const MatrixSystem& matrixSystem, AlgebraicOperand& input) {
            // Get matrix, or throw
            const auto& symMatrix = input.to_matrix(matrixSystem);

            if (!symMatrix.is_monomial()) {
                throw_error(matlabEngine, errors::internal_error,
                            "Polynomial matrix multiplication not yet supported");
            }

            if (!symMatrix.has_operator_matrix()) {
                throw_error(matlabEngine, errors::bad_param,
                            "Cannot multiply matrix that is not explicitly defined by monomial operators.");
            }
            return dynamic_cast<const MonomialMatrix&>(symMatrix);
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        matrix_by_polynomial(matlab::engine::MATLABEngine& matlabEngine,
                             MultiplyParams &input, MatrixSystem &matrixSystem) {

            // Get read lock on matrix system
            auto write_lock = matrixSystem.get_write_lock();

            const auto& matrix = input_to_monomial_matrix(matlabEngine, matrixSystem, input.lhs);
            const auto polynomial = input.rhs.to_polynomial(matrixSystem);

            try {
                if (polynomial.is_monomial() && (!polynomial.empty())) {
                    return matrix.post_multiply(polynomial.back(), matrixSystem.Symbols(),
                                                Multithreading::MultiThreadPolicy::Optional);
                } else {
                    return matrix.post_multiply(polynomial, matrixSystem.polynomial_factory(), matrixSystem.Symbols(),
                                                Multithreading::MultiThreadPolicy::Optional);
                }
            } catch(const Moment::errors::cannot_multiply_exception& cme) {
                throw_error(matlabEngine, errors::internal_error, cme.what());
            }
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        polynomial_by_matrix(matlab::engine::MATLABEngine& matlabEngine,
                             MultiplyParams &input, MatrixSystem &matrixSystem) {

            // Get read lock on matrix system
            auto write_lock = matrixSystem.get_write_lock();

            const auto polynomial = input.lhs.to_polynomial(matrixSystem);
            const auto& matrix = input_to_monomial_matrix(matlabEngine, matrixSystem, input.rhs);

            try {
                if (polynomial.is_monomial() && (!polynomial.empty())) {
                    return matrix.pre_multiply(polynomial.back(), matrixSystem.Symbols(),
                                               Multithreading::MultiThreadPolicy::Optional);
                } else {
                    return matrix.pre_multiply(polynomial, matrixSystem.polynomial_factory(), matrixSystem.Symbols(),
                                               Multithreading::MultiThreadPolicy::Optional);
                }
            } catch(const Moment::errors::cannot_multiply_exception& cme) {
                throw_error(matlabEngine, errors::internal_error, cme.what());
            }
        }

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        do_multiplication(matlab::engine::MATLABEngine& matlabEngine,
                          MultiplyParams &input, MatrixSystem &matrixSystem) {
            switch (input.lhs.type) {
                case AlgebraicOperand::InputType::MatrixID:
                    switch (input.rhs.type) {
                        case AlgebraicOperand::InputType::MatrixID:
                            throw_error(matlabEngine, errors::internal_error, "Matrix RHS not yet implemented.");
                        case AlgebraicOperand::InputType::Monomial:
                        case AlgebraicOperand::InputType::Polynomial:
                            return matrix_by_polynomial(matlabEngine, input, matrixSystem);
                        default:
                        case AlgebraicOperand::InputType::Unknown:
                            throw_error(matlabEngine, errors::bad_param, "Cannot multiply unknown RHS type.");
                    }
                    break;
                case AlgebraicOperand::InputType::Monomial:
                case AlgebraicOperand::InputType::Polynomial:
                    switch (input.rhs.type) {
                        case AlgebraicOperand::InputType::MatrixID:
                            return polynomial_by_matrix(matlabEngine, input, matrixSystem);
                        case AlgebraicOperand::InputType::Monomial:
                        case AlgebraicOperand::InputType::Polynomial:
                            throw_error(matlabEngine, errors::internal_error, "Polynomial RHS not yet implemented.");
                            break;
                        default:
                        case AlgebraicOperand::InputType::Unknown:
                            throw_error(matlabEngine, errors::bad_param, "Cannot multiply unknown RHS type.");
                    }
                    break;
                default:
                case AlgebraicOperand::InputType::Unknown:
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
            : SortedInputs{std::move(structuredInputs)},
              matrix_system_key{matlabEngine}, lhs{matlabEngine, "LHS"}, rhs{matlabEngine, "RHS"} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Check type of LHS input
        this->lhs.parse_input(this->inputs[1]);

        // Check type of RHS input
        this->rhs.parse_input(this->inputs[2]);

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
        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.matrix_system_key(this->storageManager);
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
