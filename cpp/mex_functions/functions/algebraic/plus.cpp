/**
 * plus.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "plus.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "storage_manager.h"

#include "scenarios/context.h"
#include "export/export_operator_matrix.h"
#include "export/export_polynomial.h"

#include <span>

namespace Moment::mex::functions {

    namespace {

        void output_polynomials(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                                IOArgumentRange& output, PlusParams::OutputMode output_mode,
                                matlab::data::ArrayDimensions output_shape,
                                const std::span<const Polynomial> output_poly) {

            // Attempt to infer if output is a monomial object
            const bool detect_if_monomial = output.size() >= 2;
            bool is_monomial = false;
            if (detect_if_monomial) {
                is_monomial = std::all_of(output_poly.begin(), output_poly.end(), [](const Polynomial& poly) {
                    return poly.is_monomial();
                });
            }

            // Export polynomials
            matlab::data::ArrayFactory factory;
            PolynomialExporter exporter{matlabEngine, factory,
                                        matrixSystem.Context(), matrixSystem.Symbols(),
                                        matrixSystem.polynomial_factory().zero_tolerance};
            switch (output_mode) {
                case PlusParams::OutputMode::String: {
                    matlab::data::StringArray string_out
                        = factory.createArray<matlab::data::MATLABString>(std::move(output_shape));

                    std::transform(output_poly.begin(), output_poly.end(), string_out.begin(),
                                   [&exporter](const Polynomial &poly) -> matlab::data::MATLABString {
                                       return exporter.string(poly);
                                   });
                    output[0] = std::move(string_out);
                } break;
                case PlusParams::OutputMode::SymbolCell: {
                    matlab::data::CellArray cell_out = factory.createCellArray(std::move(output_shape));
                    std::transform(output_poly.begin(), output_poly.end(), cell_out.begin(),
                                   [&exporter](const Polynomial &poly) -> matlab::data::CellArray {
                                       return exporter.symbol_cell(poly);
                                   });
                    output[0] = std::move(cell_out);
                } break;
                case PlusParams::OutputMode::SequencesWithSymbolInfo: {
                    if (is_monomial) {
                        assert(detect_if_monomial);
                        auto fms = exporter.monomial_sequence_cell_vector(output_poly, output_shape, true);
                        output[0] = fms.move_to_cell(factory);
                    } else {
                        output[0] = exporter.sequence_cell_vector(output_poly, output_shape, true);
                    }
                } break;
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output format for plus.");
            }

            // Write if output object is purely monomial.
            if (detect_if_monomial) {
                assert (output.size() >= 2);
                output[1] = factory.createScalar<bool>(is_monomial);
            }
        }

        void output_matrix(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                           IOArgumentRange& output, PlusParams::OutputMode output_mode,
                           const PolynomialMatrix& matrix) {

            // Export polynomial matrix
            OperatorMatrixExporter ome{matlabEngine, matrixSystem};

            switch (output_mode) {
                case PlusParams::OutputMode::String:
                    output[0] = ome.sequence_strings(matrix);
                    break;
                case PlusParams::OutputMode::SymbolCell:
                    output[0] = ome.symbol_cell(matrix);
                    break;
                case PlusParams::OutputMode::SequencesWithSymbolInfo: {
                    output[0] = ome.polynomials(matrix);
                } break;
                case PlusParams::OutputMode::Unknown:
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output format for plus.");
            }
        }

        void save_and_output(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                             IOArgumentRange& output, std::unique_ptr<PolynomialMatrix> matrix) {

            const size_t dimension = matrix->Dimension();
            const bool is_hermitian = matrix->Hermitian(); // (Monomial = false automatically)
            auto write_lock = matrixSystem.get_write_lock();
            const ptrdiff_t matrix_index = matrixSystem.push_back(write_lock, std::move(matrix));
            write_lock.unlock();

            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar<int64_t>(matrix_index);
            output[1] = factory.createScalar<size_t>(dimension);
            output[2] = factory.createScalar<bool>(false); // Monomial = false automatically.
            output[3] = factory.createScalar<bool>(is_hermitian);
            return;
        }

        void add_matrix_matrix(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                               IOArgumentRange& output, const AlgebraicOperand& lhs, const AlgebraicOperand& rhs,
                               PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            auto& matrixLHS = lhs.to_matrix(matrixSystem);
            auto& matrixRHS = rhs.to_matrix(matrixSystem);

            // Check size compatibility for many<->many
            if (matrixLHS.Dimension() != matrixRHS.Dimension()) {
                throw_error(matlabEngine, errors::bad_param, "Matrix operand dimensions do not match");
            }

            // Do addition
            auto added_matrix_ptr = matrixLHS.add(matrixRHS, matrixSystem.polynomial_factory(),
                                                  Multithreading::MultiThreadPolicy::Optional);

            // Save and output, if matrix ID mode
            if (output_mode == PlusParams::OutputMode::MatrixID) {
                read_lock.unlock();
                save_and_output(matlabEngine, matrixSystem, output, std::move(added_matrix_ptr));
                return;
            }

            // Do output
            output_matrix(matlabEngine, matrixSystem, output, output_mode,  *added_matrix_ptr);
        }

        void add_one_matrix(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                            IOArgumentRange& output, AlgebraicOperand& lhs, AlgebraicOperand& rhs,
                            PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            auto polyLHS = lhs.to_polynomial(matrixSystem, true);
            auto& matrixRHS = rhs.to_matrix(matrixSystem);

            // Do addition
            auto added_matrix_ptr = matrixRHS.add(polyLHS, matrixSystem.polynomial_factory(),
                                                  Multithreading::MultiThreadPolicy::Optional);

            // Save and output, if matrix ID mode
            if (output_mode == PlusParams::OutputMode::MatrixID) {
                read_lock.unlock();
                save_and_output(matlabEngine, matrixSystem, output, std::move(added_matrix_ptr));
                return;
            }

            // Do output
            output_matrix(matlabEngine, matrixSystem, output, output_mode, *added_matrix_ptr);
        }

        void add_many_matrix(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                             IOArgumentRange& output, AlgebraicOperand& lhs, AlgebraicOperand& rhs,
                             PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            auto polysLHS = lhs.to_polynomial_array(matrixSystem, true);
            auto& matrixRHS = rhs.to_matrix(matrixSystem);

            const auto& poly_factory =  matrixSystem.polynomial_factory();

            // Check size compatibility for many<->many
            if ((lhs.shape.size() != 2) || (lhs.shape[0] != matrixRHS.Dimension())
                                        || (lhs.shape[1] != matrixRHS.Dimension())) {
                throw_error(matlabEngine, errors::bad_param, "Polynomial dimensions do not match matrix dimensions.");
            }

            // Move constructed data into polynomial matrix object
            auto matrixLHS_data = std::make_unique<PolynomialMatrix::MatrixData>(matrixRHS.Dimension(),
                                                                                std::move(polysLHS));

            PolynomialMatrix matrixLHS{matrixSystem.Context(), matrixSystem.Symbols(),
                                       poly_factory.zero_tolerance, std::move(matrixLHS_data)};

            auto added_matrix_ptr = matrixLHS.add(matrixRHS, poly_factory, Multithreading::MultiThreadPolicy::Optional);

            // Save and output, if matrix ID mode
            if (output_mode == PlusParams::OutputMode::MatrixID) {
                read_lock.unlock();
                save_and_output(matlabEngine, matrixSystem, output, std::move(added_matrix_ptr));
                return;
            }

            // Do output
            output_matrix(matlabEngine, matrixSystem, output, output_mode, *added_matrix_ptr);
        }

        void add_one_one(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                         IOArgumentRange& output, AlgebraicOperand& lhs, AlgebraicOperand& rhs,
                         PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            auto polyOutput = lhs.to_polynomial(matrixSystem, true);
            auto polyRHS = rhs.to_polynomial(matrixSystem, true);

            // Do addition
            const auto& poly_factory = matrixSystem.polynomial_factory();
            poly_factory.append(polyOutput, polyRHS);

            // Output
            output_polynomials(matlabEngine, matrixSystem, output, output_mode,
                               matlab::data::ArrayDimensions{1, 1},
                               std::span<const Polynomial>(&polyOutput, 1));
        }

        void add_one_many(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                         IOArgumentRange& output, AlgebraicOperand& lhs, AlgebraicOperand& rhs,
                         PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            const auto polyLHS = lhs.to_polynomial(matrixSystem, true);
            auto polysOutput = rhs.to_polynomial_array(matrixSystem, true);

            // Do addition
            const auto& poly_factory = matrixSystem.polynomial_factory();
            for (auto& polyOut : polysOutput) {
                poly_factory.append(polyOut, polyLHS);
            }

            // Output
            output_polynomials(matlabEngine, matrixSystem, output, output_mode,
                               rhs.shape, polysOutput);
        }

        void add_many_many(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem& matrixSystem,
                           IOArgumentRange& output, AlgebraicOperand& lhs, AlgebraicOperand& rhs,
                           PlusParams::OutputMode output_mode) {
            // Read inputs
            auto read_lock = matrixSystem.get_read_lock();
            auto polysOutput = lhs.to_polynomial_array(matrixSystem, true);
            auto polysRHS = rhs.to_polynomial_array(matrixSystem, true);

            // Check size compatibility for many<->many
            if (!std::equal(lhs.shape.cbegin(), lhs.shape.cend(), rhs.shape.cbegin(), rhs.shape.cend())) {
                throw_error(matlabEngine, errors::bad_param,
                            "Argument dimensions must match (or one element must be a scalar) to use plus.");
            }
            assert(polysOutput.size() == polysRHS.size());

            // Do addition
            const auto& poly_factory = matrixSystem.polynomial_factory();
            for (size_t n = 0; n < polysOutput.size(); ++n) {
                poly_factory.append(polysOutput[n], polysRHS[n]);
            }

            // Output
            output_polynomials(matlabEngine, matrixSystem, output, output_mode,
                               rhs.shape, polysOutput);
        }
    }

    PlusParams::PlusParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)}, lhs{matlabEngine, "LHS"}, rhs{matlabEngine, "RHS"} {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(this->matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Get left operand
        this->lhs.parse_input(this->inputs[1]);

        // Get right operand
        this->rhs.parse_input(this->inputs[2]);

        // How do we output?
        if (this->flags.contains(u"strings")) {
            this->output_mode = OutputMode::String;
        } else if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::SequencesWithSymbolInfo;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::SymbolCell;
        } else {
            this->output_mode = OutputMode::MatrixID;
            if ((this->lhs.type != AlgebraicOperand::InputType::MatrixID)
                && (this->rhs.type != AlgebraicOperand::InputType::MatrixID)) {
                throw_error(matlabEngine, errors::bad_param,
                            "At least one operand must be a matrix for matrix index output.");
            }
        }
    }

    void Plus::extra_input_checks(PlusParams& input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Plus::Plus(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 4;

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"strings");
        this->flag_names.emplace(u"index");

        this->mutex_params.add_mutex({u"index", u"symbols", u"sequences", u"strings"});
    }

    void Plus::operator()(IOArgumentRange output, PlusParams &input) {
        // Check output mode
        if (input.output_mode == PlusParams::OutputMode::MatrixID) {
            if (output.size() < 4) {
                throw_error(matlabEngine, errors::too_few_outputs,
                            "Must provide 4 outputs for operator matrix index output mode.");
            }
        } else {
            if (output.size() > 2) {
                throw_error(matlabEngine, errors::too_many_outputs,
                            "Too many outputs provided when not in operator matrix index output mode.");
            }
        }

        // First, get matrix system
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        MatrixSystem& matrixSystem = *matrixSystemPtr;

        // Switch between various additions
        switch (input.lhs.type) {
            case AlgebraicOperand::InputType::MatrixID:
                switch (input.rhs.type) {
                    case AlgebraicOperand::InputType::MatrixID:
                        add_matrix_matrix(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::Polynomial:
                        add_one_matrix(matlabEngine, matrixSystem, output, input.rhs, input.lhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::PolynomialArray:
                        add_many_matrix(matlabEngine, matrixSystem, output, input.rhs, input.lhs, input.output_mode);
                        break;
                    default:
                    case AlgebraicOperand::InputType::Unknown:
                        throw_error(this->matlabEngine, errors::internal_error, "Unknown RHS operand.");
                }
                break;
            case AlgebraicOperand::InputType::Polynomial:
                switch (input.rhs.type) {
                    case AlgebraicOperand::InputType::MatrixID:
                        add_one_matrix(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::Polynomial:
                        add_one_one(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::PolynomialArray:
                        add_one_many(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    default:
                    case AlgebraicOperand::InputType::Unknown:
                        throw_error(this->matlabEngine, errors::internal_error, "Unknown RHS operand.");
                }
                break;
            case AlgebraicOperand::InputType::PolynomialArray:
                switch (input.rhs.type) {
                    case AlgebraicOperand::InputType::MatrixID:
                        add_many_matrix(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::Polynomial:
                        add_one_many(matlabEngine, matrixSystem, output, input.rhs, input.lhs, input.output_mode);
                        break;
                    case AlgebraicOperand::InputType::PolynomialArray:
                        add_many_many(matlabEngine, matrixSystem, output, input.lhs, input.rhs, input.output_mode);
                        break;
                    default:
                    case AlgebraicOperand::InputType::Unknown:
                        throw_error(this->matlabEngine, errors::internal_error, "Unknown RHS operand.");
                }
                break;
            default:
            case AlgebraicOperand::InputType::Unknown:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown LHS operand.");
        }
    }
}