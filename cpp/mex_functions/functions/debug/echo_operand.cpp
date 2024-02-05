/**
 * echo_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "echo_operand.h"
#include "storage_manager.h"

#include "eigen/export_eigen_dense.h"
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
        this->max_outputs = 2;
        this->flag_names.emplace(u"symbolic");
        this->flag_names.emplace(u"to_symbols");
    }


    namespace {
        inline void output_matrix_system_id(std::ostream& os, const uint64_t key, const MatrixSystem& system) {
            os << "Matrix system: 0x" << std::hex << key << std::dec << " -> "
               << system.system_type_name() << ".\n";
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

        void output_monomial(matlab::engine::MATLABEngine& engine,
                                   IOArgumentRange& output, const EchoOperandParams& input,
                                   const bool print_output, const MatrixSystem& system,
                                   const RawPolynomial& raw_polynomial) {
            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                auto fps = pe.sequences(raw_polynomial);
                output[0] = fps.move_to_cell(factory);
            }

            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                ss << input.operand;
                ss << "Monomial: " << raw_polynomial.to_string(system.Context()) << ".\n";
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
                ss << input.operand;
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
                ss << input.operand;
                cSS << "Symbolic polynomial: " << polynomial << ".\n";
                print_to_console(engine, ss.str());
            }
        }


        void output_monomial_array(matlab::engine::MATLABEngine& engine,
                                   IOArgumentRange& output, const EchoOperandParams& input,
                                   const bool print_output, const MatrixSystem& system,
                                   const std::span<const RawPolynomial> raw_polynomials) {

            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                PolynomialExporter pe{engine, factory, system.Context(), system.Symbols(),
                                      system.polynomial_factory().zero_tolerance};
                auto fps = pe.monomial_sequence_cell_vector(raw_polynomials, input.operand.shape);
                output[0] = fps.move_to_cell(factory);
            }

            if (print_output) {
                std::stringstream ss;
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                ss << input.operand;
                for (const auto& raw_poly : raw_polynomials) {
                    ss << raw_poly.to_string(system.Context()) << "\n";
                }
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
                output_matrix_system_id(ss, input.matrix_system_key.value(), system);
                ss << input.operand;
                for (const auto& raw_poly : raw_polynomials) {
                    ss << raw_poly.to_string(system.Context()) << "\n";
                }
                print_to_console(engine, ss.str());
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
                ss << input.operand;
                print_to_console(engine, ss.str());
                ContextualOS cSS{ss, system.Context(), system.Symbols()};
                for (const auto& poly : polynomials) {
                    cSS << poly << "\n";
                }
            }
        }

        void output_symbolic(matlab::engine::MATLABEngine& engine,
                             IOArgumentRange& output, EchoOperandParams& input,
                             const bool print_output, const MatrixSystem& matrix_system) {

            // Try and parse symbolic cases:
            const bool is_scalar = input.operand.is_scalar();
            const bool output_as_monomial = !input.parse_to_symbols && input.operand.is_monomial();
            if (is_scalar) {
                if (input.parse_to_symbols) {
                    auto parsed_poly = input.operand.to_polynomial(matrix_system);
                    output_full_polynomial(engine, output, input, print_output, matrix_system, parsed_poly);
                } else {
                    auto parsed_raw_poly = input.operand.to_raw_polynomial(matrix_system);
                    if (output_as_monomial) {
                        output_monomial(engine, output, input, print_output, matrix_system, parsed_raw_poly);
                    } else {
                        output_raw_polynomial(engine, output, input, print_output, matrix_system, parsed_raw_poly);
                    }
                }
            } else {
                if (input.parse_to_symbols) {
                    auto parsed_polys = input.operand.to_polynomial_array(matrix_system);
                    output_full_polynomial_array(engine, output, input, print_output,
                                                 matrix_system, parsed_polys);
                } else {
                    auto parsed_raw_polys = input.operand.to_raw_polynomial_array(matrix_system);
                    if (output_as_monomial) {
                        output_monomial_array(engine, output, input, print_output,
                                              matrix_system, parsed_raw_polys);
                    } else {
                        output_raw_polynomial_array(engine, output, input, print_output,
                                                    matrix_system, parsed_raw_polys);
                    }
                }
            }

            // Write monomial status of output
            if (output.size() >= 2) {
                matlab::data::ArrayFactory factory;
                output[1] = factory.createScalar<bool>(output_as_monomial);
            }
        }

        void output_scalar_value(matlab::engine::MATLABEngine& engine, IOArgumentRange& output,
                                 const AlgebraicOperand& operand, const bool print_output) {
            if (print_output) {
                std::stringstream ss;
                ss << operand;

                if (operand.type == AlgebraicOperand::InputType::RealNumber) {
                    double value = operand.raw_scalar();
                    ss << ": " << value << "\n";
                } else {
                    std::complex<double> value = operand.raw_complex_scalar();
                    ss << ": " << value.real() << " + " << value.imag() << "i\n";
                }

                print_to_console(engine, ss.str());
            }

            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                if (operand.type == AlgebraicOperand::InputType::RealNumber) {
                    double value = operand.raw_scalar();
                    output[0] = factory.createScalar<double>(value);
                } else {
                    assert(operand.type == AlgebraicOperand::InputType::ComplexNumber);

                    std::complex<double> value = operand.raw_complex_scalar();
                    output[0] = factory.createScalar<std::complex<double>>(value);
                }
            }
        }

        void output_numeric_array(matlab::engine::MATLABEngine& engine, IOArgumentRange& output,
                                  const AlgebraicOperand& operand, const bool print_output) {
            if (print_output) {
                std::stringstream ss;
                ss << operand << ":\n";
                if (operand.type == AlgebraicOperand::InputType::RealNumberArray) {
                    const auto& matrix = operand.raw_numeric_array();
                    for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                        ss << " " << *iter << "\n";
                    }
                } else {
                    const auto& matrix = operand.raw_complex_numeric_array();
                    for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                        ss << " " << iter->real() << " + " << iter->imag() << "i\n";
                    }
                }
                print_to_console(engine, ss.str());
            }

            if (output.size() >= 1) {
                matlab::data::ArrayFactory factory;
                if (operand.type == AlgebraicOperand::InputType::RealNumberArray) {
                    const auto& value = operand.raw_numeric_array();
                    output[0] = export_eigen_dense(engine, factory, value);
                } else {
                    assert(operand.type == AlgebraicOperand::InputType::ComplexNumberArray);
                    const auto& value = operand.raw_complex_numeric_array();
                    output[0] = export_eigen_dense(engine, factory, value);
                }
            }
        }

        void output_numeric(matlab::engine::MATLABEngine& engine,
                            IOArgumentRange& output, EchoOperandParams& input,
                            const bool print_output, const MatrixSystem& matrix_system) {
            switch (input.operand.type) {
                case AlgebraicOperand::InputType::RealNumber:
                case AlgebraicOperand::InputType::ComplexNumber:
                    output_scalar_value(engine, output, input.operand, print_output);
                    break;
                case AlgebraicOperand::InputType::RealNumberArray:
                case AlgebraicOperand::InputType::ComplexNumberArray:
                    output_numeric_array(engine, output, input.operand, print_output);
                    break;
                default:
                    throw_error(engine, errors::internal_error, "Unexpected input type.");
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
                output_symbolic(this->matlabEngine, output, input, print_output, matrix_system);
                return;

            case AlgebraicOperand::InputType::RealNumber:
            case AlgebraicOperand::InputType::RealNumberArray:
            case AlgebraicOperand::InputType::ComplexNumber:
            case AlgebraicOperand::InputType::ComplexNumberArray:
                output_numeric(this->matlabEngine, output, input, print_output, matrix_system);
                return;

            default: // Catch bad input cases
            case AlgebraicOperand::InputType::Unknown:
                throw_error(matlabEngine, errors::bad_param, "Unknown algebraic operand!");
        }
    }


}