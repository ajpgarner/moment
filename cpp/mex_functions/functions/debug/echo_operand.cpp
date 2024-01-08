/**
 * echo_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "echo_operand.h"
#include "storage_manager.h"

#include "export/export_polynomial.h"
#include "utilities/read_as_scalar.h"

#include "dictionary/raw_polynomial.h"
#include "scenarios/contextual_os.h"
#include "symbolic/polynomial.h"

#include <span>

namespace Moment::mex::functions  {
        EchoOperandParams::EchoOperandParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)},
            matrix_system_key{matlabEngine},  operand{matlabEngine, "Operand"} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Get operand
        //const bool expect_symbols = this->flags.contains(u"symbolic");
        this->operand.parse_input(this->inputs[1]);

        // Parse to symbols (default: only parse to symbols if symbols supplied as input).
        if (this->flags.contains(u"to_symbols")) {
            this->parse_to_symbols = true;
        } else {
            this->parse_to_symbols = false;
        }
    }

    EchoOperand::EchoOperand(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 0;
        this->max_outputs = 1;
        this->flag_names.emplace(u"symbolic");
        this->flag_names.emplace(u"to_symbols");
    }


    namespace {
        inline void output_matrix_system_id(std::ostream& os, const uint64_t key, const MatrixSystem& system) {
            os << "Matrix system: 0x" << std::hex << key << std::dec << " -> "
               << system.system_type_name() << ".\n";
        }

        inline void output_operand_summary(std::ostream& os, const AlgebraicOperand& operand) {
            if (operand.is_scalar()) {
                os << "Scalar";
            } else if (operand.shape.empty()) {
                os << "Empty";
            } else {
                bool once = false;
                for (const auto dim : operand.shape) {
                    if (once) {
                        os << " x ";
                    }
                    os << dim;
                    once = true;
                }
            }
            os << " ";

            switch (operand.type) {
                case AlgebraicOperand::InputType::Monomial:
                case AlgebraicOperand::InputType::MonomialArray:
                    os << "monomial";
                    break;
                case AlgebraicOperand::InputType::Polynomial:
                case AlgebraicOperand::InputType::PolynomialArray:
                    os << "polynomial";
                    break;
                case AlgebraicOperand::InputType::EmptyObject:
                    os << "operand";
                    break;
                default:
                    os << "[unexpected: " << static_cast<int>(operand.type) << "]";
                    break;
            }
            os << " (input as: ";
            switch (operand.format) {

                case AlgebraicOperand::InputFormat::Unknown:
                    os << "unknown";
                    break;
                case AlgebraicOperand::InputFormat::Number:
                    os << "number";
                    break;
                case AlgebraicOperand::InputFormat::SymbolCell:
                    os << "symbol cell";
                    break;
                case AlgebraicOperand::InputFormat::OperatorCell:
                    os << "operator cell";
                    break;
                default:
                    os << "[unexpected: " << static_cast<int>(operand.type) << "]";
            }
            os << ").\n";
        }


        void output_empty(matlab::engine::MATLABEngine& engine,
                          IOArgumentRange& output, const EchoOperandParams& input,
                          const bool print_output, const MatrixSystem& system) {

            matlab::data::ArrayFactory factory;
            if (output.size() >= 1) {
                output[0] = factory.createScalar<bool>(false);
            }
            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                ss << "Empty algebraic operand.\n";
                print_to_console(engine, ss.str());
            }
        }

        void output_matrix_key(matlab::engine::MATLABEngine& engine,
                               IOArgumentRange& output, const EchoOperandParams& input,
                               const bool print_output, const MatrixSystem& system) {

            matlab::data::ArrayFactory factory;
            if (output.size() >= 1) {
                output[0] = factory.createScalar<uint64_t>(static_cast<uint64_t>(input.operand.matrix_key()));
            }
            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                ss << "Matrix input, index: " << std::dec << input.operand.matrix_key() << ".\n";
                print_to_console(engine, ss.str());
            }
        }

        void output_raw_polynomial(matlab::engine::MATLABEngine& engine,
                                   IOArgumentRange& output, const EchoOperandParams& input,
                                   const bool print_output, const MatrixSystem& system,
                                   const RawPolynomial& raw_polynomial) {
            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                auto fme = pe.sequences(raw_polynomial);
                auto cell_out = factory.createCellArray(matlab::data::ArrayDimensions{1, 1});
                *cell_out.begin() = fme.move_to_cell(factory);
                output[0] = std::move(cell_out);
            }

            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                output_operand_summary(ss, input.operand);
                ss << "Raw polynomial: " << raw_polynomial.to_string(system.Context()) << ".\n";
                print_to_console(engine, ss.str());
            }
        }

        void output_full_polynomial(matlab::engine::MATLABEngine& engine,
                                    IOArgumentRange& output, const EchoOperandParams& input,
                                    const bool print_output, const MatrixSystem& system,
                                    const Polynomial& polynomial) {
            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                auto fme = pe.sequences(polynomial, true);
                auto cell_out = factory.createCellArray(matlab::data::ArrayDimensions{1, 1});
                *cell_out.begin() = fme.move_to_cell(factory);
                output[0] = std::move(cell_out);
            }

            if (print_output) {
                std::stringstream ss;
                ContextualOS cSS{ss, system.Context(), system.Symbols()};
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                output_operand_summary(ss, input.operand);
                cSS << "Symbolic polynomial: " << polynomial << ".\n";
                print_to_console(engine, ss.str());
            }
        }


        void output_raw_polynomial_array(matlab::engine::MATLABEngine& engine,
                                   IOArgumentRange& output, const EchoOperandParams& input,
                                   const bool print_output, const MatrixSystem& system,
                                   const std::span<const RawPolynomial> raw_polynomials) {

            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                output[0] = pe.sequence_cell_vector(raw_polynomials, input.operand.shape);
            }

            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);output_operand_summary(ss, input.operand);
                print_to_console(engine, ss.str());
                for (const auto& raw_poly : raw_polynomials) {
                    ss << raw_poly.to_string(system.Context()) << "\n";
                }
            }
        }

        void output_full_polynomial_array(matlab::engine::MATLABEngine& engine,
                                          IOArgumentRange& output, const EchoOperandParams& input,
                                          const bool print_output, const MatrixSystem& system,
                                          const std::span<const Polynomial> polynomials) {
            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                output[0] = pe.sequence_cell_vector(polynomials, input.operand.shape);
            }

            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                output_operand_summary(ss, input.operand);
                print_to_console(engine, ss.str());
                ContextualOS cSS{ss, system.Context(), system.Symbols()};
                for (const auto& poly : polynomials) {
                    cSS << poly << "\n";
                }
            }
        }
    }



    void EchoOperand::operator()(IOArgumentRange output, EchoOperandParams &input) {
        std::shared_ptr<MatrixSystem> matrix_system_ptr = input.matrix_system_key(this->storageManager);
        const auto& matrix_system = *matrix_system_ptr;
        const bool print_output = this->verbose || (output.size() == 0);

        // First, handle 'trivial' cases
        switch(input.operand.type) {
            case AlgebraicOperand::InputType::MatrixID:
                output_matrix_key(this->matlabEngine, output, input, print_output, matrix_system);
                return;

            case AlgebraicOperand::InputType::EmptyObject:
                output_empty(this->matlabEngine,  output, input, print_output, matrix_system);
                return;

            case AlgebraicOperand::InputType::Monomial:
            case AlgebraicOperand::InputType::MonomialArray:
            case AlgebraicOperand::InputType::Polynomial:
            case AlgebraicOperand::InputType::PolynomialArray:
                break; // Move on...

            default: // Catch bad input cases
            case AlgebraicOperand::InputType::Unknown:
                throw_error(matlabEngine, errors::bad_param, "Unknown algebraic operand!");
        }

        // Now, try and parse remaining cases:
        const bool is_scalar = input.operand.is_scalar();
        if (is_scalar) {
            if (input.parse_to_symbols) {
                auto parsed_poly = input.operand.to_polynomial(matrix_system);
                output_full_polynomial(this->matlabEngine, output, input, print_output, matrix_system, parsed_poly);
            } else {
                auto parsed_raw_poly = input.operand.to_raw_polynomial(matrix_system);
                output_raw_polynomial(this->matlabEngine, output, input, print_output, matrix_system, parsed_raw_poly);
            }
        } else {
            if (input.parse_to_symbols) {
                auto parsed_polys = input.operand.to_polynomial_array(matrix_system);
                output_full_polynomial_array(this->matlabEngine, output, input, print_output,
                                             matrix_system, parsed_polys);
            } else {
                auto parsed_raw_polys = input.operand.to_raw_polynomial_array(matrix_system);
                output_raw_polynomial_array(this->matlabEngine, output, input, print_output,
                                            matrix_system, parsed_raw_polys);
            }
        }
    }


}