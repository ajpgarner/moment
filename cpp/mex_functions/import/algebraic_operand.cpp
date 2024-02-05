/**
 * algebraic_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "algebraic_operand.h"

#include "error_codes.h"
#include "eigen/read_eigen_dense.h"

#include "matrix/value_matrix.h"
#include "matrix_system/matrix_system.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include <ostream>
#include <sstream>

namespace Moment::mex {

    void AlgebraicOperand::parse_input(const matlab::data::Array& input) {
        // Check type of LHS input
        switch (input.getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::COMPLEX_SINGLE:
            case matlab::data::ArrayType::COMPLEX_DOUBLE:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
            case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE:
                this->parse_as_numeric_data(input);
                return;
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                this->parse_as_matrix_key(input);
                return;
            case matlab::data::ArrayType::CELL:
                this->parse_cell(input);
                return;
            default:
            case matlab::data::ArrayType::UNKNOWN: {
                std::stringstream ss;
                ss << name << " was not a valid operand.";
                throw_error(matlabEngine, errors::bad_param, ss.str());
            }
            return;
        }
    }

    void AlgebraicOperand::parse_as_matrix_key(const matlab::data::Array& raw_input) {
        this->type = InputType::MatrixID;
        this->format = InputFormat::Integer;
        this->shape = std::vector<size_t>{0, 0};
        this->raw.emplace<0>(read_as_uint64(matlabEngine, raw_input));
    }

    void AlgebraicOperand::parse_as_numeric_data(const matlab::data::Array& raw_input) {
        // Type: number
        this->format = InputFormat::NumericData;
        const bool is_scalar = raw_input.getNumberOfElements() == 1;
        //? InputType::NumberArray : InputType::Number;

        // Copy object dimensions
        const auto dimensions = raw_input.getDimensions();
        this->shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));
        if (this->shape.size() != 2) {
            throw_error(this->matlabEngine, errors::bad_param, "Numeric data input must be scalar or matrix.");
        }

        // Real, or complex?
        const bool is_complex = ((raw_input.getType() == matlab::data::ArrayType::COMPLEX_DOUBLE)
                                || (raw_input.getType() == matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE));


        if (is_scalar) { // If scalar, don't bother with Eigen...
            if (is_complex) {
                this->type = InputType::ComplexNumber;
                this->raw.emplace<4>(read_as_complex_double(this->matlabEngine, raw_input));
            } else {
                this->type = InputType::RealNumber;
                this->raw.emplace<3>(read_as_double(this->matlabEngine, raw_input));
            }
        } else {
            if (is_complex) {
                this->type = InputType::ComplexNumberArray;
                this->raw.emplace<6>(read_eigen_dense_complex(this->matlabEngine, raw_input));
            } else {
                this->type = InputType::RealNumberArray;
                this->raw.emplace<5>(read_eigen_dense(this->matlabEngine, raw_input));
            }
        }
    }

    void AlgebraicOperand::parse_cell(const matlab::data::Array& raw_input) {
        // Empty input -> flag as such; cannot infer anything more.
        if (raw_input.isEmpty()) {
            this->type = InputType::EmptyObject;
            this->shape = std::vector<size_t>{0, 0};
            return;
        }

        // Cast to cell array (MATLAB documentation indicates this makes a reference, not a copy).
        const matlab::data::CellArray cell_input = raw_input;

        // Copy object dimensions
        const auto dimensions = cell_input.getDimensions();
        this->shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));

        // Iterate, until we can determine object type:
        this->format = InputFormat::Unknown;
        size_t outer_index = 0;
        for (auto iter = cell_input.begin(); iter != cell_input.end(); ++iter) {
            matlab::data::Array contained_object = *iter;
            if ((contained_object.getType() != matlab::data::ArrayType::CELL)) {
                std::stringstream errSS;
                errSS << name << " element " << (outer_index+1) << " was not a cell array.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }

            // Try to guess type from this object, if not yet determined.
            if (this->format == InputFormat::Unknown) {
                matlab::data::CellArray scalar_object = contained_object;
                this->format = this->infer_format_from_scalar_object(scalar_object, outer_index);
            }
            ++outer_index;
        }

        // Note target type:
        this->type = this->infer_type_from_valid_cell(cell_input);

        // Now, parse based on format identified
        switch (this->format) {
            case InputFormat::Unknown:
                // All scalar objects were empty, so can default to symbol cell create zero polynomials.
                this->format = InputFormat::SymbolCell;
                [[fallthrough]];
            case InputFormat::SymbolCell:
                this->parse_as_symbol_cell(cell_input);
                break;
            case InputFormat::OperatorCell:
                this->parse_as_operator_cell(cell_input);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Bad deduced format.");
        }
    }

    AlgebraicOperand::InputFormat
    AlgebraicOperand::infer_format_from_scalar_object(const matlab::data::CellArray& input, size_t outer_index) const {
        assert(!input.isEmpty());

        // If scalar object is empty, cannot infer its type:
        if (input.isEmpty()) {
            return InputFormat::Unknown;
        }

        // Otherwise, try to determine type from contents:
        matlab::data::Array leading_element = *input.begin();
        if ((leading_element.getType() != matlab::data::ArrayType::CELL) || (leading_element.isEmpty())) {
            std::stringstream errSS;
            errSS << name << " element " << (outer_index+1)
                  << ", sub-element 1 should be a non-empty cell array.";
            throw_error(matlabEngine, errors::bad_param, errSS.str());
        }
        matlab::data::CellArray leading_as_cell = leading_element;

        // Now, look at first element to see if an operator sequence or a symbol
        switch (leading_as_cell.begin()->getType()) {
            case matlab::data::ArrayType::INT64:
                return InputFormat::SymbolCell;
            case matlab::data::ArrayType::UINT64:
                return InputFormat::OperatorCell;
            default: {
                std::stringstream errSS;
                errSS << name << "{" << (outer_index+1) << "}{1}{1} must be either int64 or uint64.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }
        }
    }

    AlgebraicOperand::InputType
    AlgebraicOperand::infer_type_from_valid_cell(const matlab::data::CellArray& input) const {
        const size_t expected_elements = input.getNumberOfElements();
        const bool is_scalar = expected_elements == 1;
        bool is_monomial = true; // Monomial until proved otherwise
        size_t outer_index = 0;
        for (auto object_iter = input.begin(); object_iter != input.end(); ++object_iter) {
            if (object_iter->getType() != matlab::data::ArrayType::CELL) {
                std::stringstream errSS;
                errSS << name << " element " << (outer_index + 1)
                      << " must be a cell array.";
            }
            matlab::data::CellArray object_as_cell = *object_iter;
            if (object_as_cell.getNumberOfElements() != 1) {
                is_monomial = false;
            }
            ++outer_index;
        }

        if (is_monomial) {
            return is_scalar ? InputType::Monomial : InputType::MonomialArray;
        } else {
            return is_scalar ? InputType::Polynomial : InputType::PolynomialArray;
        }

    }

    void AlgebraicOperand::parse_as_operator_cell(const matlab::data::CellArray& input) {

        const size_t expected_elements = input.getNumberOfElements();

        // Prepare raw polynomial object with data
        this->raw.emplace<2>();
        auto& raw_vec = this->raw_operator_cell_data();
        raw_vec.reserve(expected_elements);

        // Each elem is an op-seq polynomial:
        for (const auto& element : input) {
            raw_vec.emplace_back(this->matlabEngine, element, this->name);
        }

    }

    void AlgebraicOperand::parse_as_symbol_cell(const matlab::data::CellArray& input) {

        const size_t expected_elements = input.getNumberOfElements();
        if (expected_elements == 0) {
            throw_error(matlabEngine, errors::bad_param, "Operand was empty, but non-empty operand was expected.");
        }

        // Prepare Polynomial object with data
        this->raw.emplace<1>();
        auto& raw_vec = this->raw_symbol_cell_data();
        raw_vec.reserve(expected_elements);

        auto read_iter = input.begin();
        while (read_iter != input.end()) {
            raw_vec.emplace_back(read_raw_polynomial_data(matlabEngine, name, *read_iter));
            ++read_iter;
        }
    }

    bool AlgebraicOperand::is_scalar() const noexcept {
        switch (this->type) {
            default:
            case InputType::Unknown:
            case InputType::MatrixID:
            case InputType::PolynomialArray:
            case InputType::MonomialArray:
            case InputType::RealNumberArray:
            case InputType::ComplexNumberArray:
                return false;
            case InputType::Monomial:
            case InputType::Polynomial:
            case InputType::RealNumber:
            case InputType::ComplexNumber:
                return true;
        }
    }

    bool AlgebraicOperand::is_monomial() const noexcept {
        switch (this->type) {
            default:
            case InputType::Unknown:
            case InputType::MatrixID:
            case InputType::Polynomial:
            case InputType::PolynomialArray:
                return false;
            case InputType::RealNumber:
            case InputType::ComplexNumber:
            case InputType::Monomial:
            case InputType::RealNumberArray:
            case InputType::ComplexNumberArray:
            case InputType::MonomialArray:
                return true;
        }
    }

    const SymbolicMatrix& AlgebraicOperand::to_matrix(const MatrixSystem& matrixSystem) const {
        if (this->type != InputType::MatrixID) {
            throw_error(matlabEngine, errors::internal_error, "Operand was not a matrix ID.");
        }

        // Get matrix, or throw
        const auto matrix_id = this->matrix_key();
        if (matrixSystem.size() <= matrix_id) {
            std::stringstream errSS;
            errSS << "Matrix with ID '" << matrix_id << "' is out of range.";
            throw_error(matlabEngine, errors::bad_param, errSS.str());
        }
        return matrixSystem[matrix_id];
    }

    namespace {
        /** Polynomial: Scalar valued or zero */
        template<typename field_t>
        Polynomial scalar_poly(field_t the_val, double zero_tolerance) noexcept {
            if (!approximately_zero(the_val, zero_tolerance)) {
                return Polynomial::Scalar(the_val);
            } else {
                return Polynomial::Zero();
            }
        }

        /** Raw polynomial: Scalar valued or zero */
        RawPolynomial scalar_raw_poly(const Context& context,
                                      std::complex<double> the_val, double zero_tolerance) noexcept {
            if (!approximately_zero(the_val, zero_tolerance)) {
                RawPolynomial output;
                output.emplace_back(OperatorSequence::Identity(context), the_val);
                return output;
            } else {
                return RawPolynomial{};
            }
        }
    }

    const Polynomial AlgebraicOperand::to_polynomial(const MatrixSystem& system, const bool assume_sorted) {
        const auto& poly_factory = system.polynomial_factory();

        // Handle variate:
        switch (this->raw.index()) {
            case 1: { // Symbol cell input
                // Error if no data
                const auto& polys = this->raw_symbol_cell_data();
                if (polys.empty()) {
                    throw_error(matlabEngine, errors::bad_param, "Polynomial input array was empty.");
                }
                const auto& first_poly_raw = polys.front();

                // Instantiate raw polynomial
                if (assume_sorted) {
                    return raw_data_to_polynomial_assume_sorted(matlabEngine, poly_factory, first_poly_raw);
                }
                return raw_data_to_polynomial(matlabEngine, poly_factory, first_poly_raw);
            }

            case 2: { // Operator cell input
                // Error if no data:
                auto& staging_polys = this->raw_operator_cell_data();
                if (staging_polys.empty()) {
                    throw_error(matlabEngine, errors::internal_error, "Polynomial input array was empty.");
                }

                auto& the_poly = staging_polys.front();

                // If we already have data, return new polynomial
                if (the_poly.ready()) {
                    return the_poly.to_polynomial(poly_factory);
                }

                // Otherwise, supply context, look for symbols and convert to polynomial (throwing if any stage fails):
                the_poly.supply_context(system.Context());
                the_poly.find_symbols(system.Symbols(), false);
                return the_poly.to_polynomial(poly_factory);
            }

            // Real number
            case 3: {
                auto val = std::get<3>(this->raw);
                return scalar_poly(val, poly_factory.zero_tolerance);
            }

            // Complex number
            case 4: {
                auto val = std::get<4>(this->raw);
                return scalar_poly(val, poly_factory.zero_tolerance);
            }

            // Real number array
            case 5: {
                const auto& matrix = std::get<5>(this->raw);
                if (matrix.size() < 1) {
                    throw_error(matlabEngine, errors::internal_error, "Input array was unexpectedly empty.");
                }
                auto val = matrix.coeff(0, 0);
                return scalar_poly(val, poly_factory.zero_tolerance);
            }

            // Complex number array
            case 6: {
                const auto& matrix = std::get<6>(this->raw);
                if (matrix.size() < 1) {
                    throw_error(matlabEngine, errors::internal_error, "Input array was unexpectedly empty.");
                }
                auto val = matrix.coeff(0, 0);
                return scalar_poly(val, poly_factory.zero_tolerance);
            }

            default:
                throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial.");
        }
    }

    const std::vector<Polynomial>
    AlgebraicOperand::to_polynomial_array(const MatrixSystem& system, bool assume_sorted) {
        const auto& poly_factory = system.polynomial_factory();

        std::vector<Polynomial> output;
        switch (this->raw.index()) {
            // Symbol cell input
            case 1: {
                const auto& raw_polys = this->raw_symbol_cell_data();
                output.reserve(raw_polys.size());
                for (const auto& raw_poly : raw_polys) {
                    if (assume_sorted) {
                        output.emplace_back(raw_data_to_polynomial_assume_sorted(matlabEngine, poly_factory, raw_poly));
                    } else {
                        output.emplace_back(raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly));
                    }
                }
            } break;

            // Operator cell input
            case 2: {
                // Error if no data:
                auto& staging_polys = this->raw_operator_cell_data();
                output.reserve(staging_polys.size());
                for (auto& raw_poly : staging_polys) {
                    if (!raw_poly.ready()) {
                        raw_poly.supply_context(system.Context());
                        raw_poly.find_symbols(system.Symbols(), false);
                    }
                    output.emplace_back(raw_poly.to_polynomial(poly_factory));

                }
            } break;

            // Real number
            case 3: {
                auto val = std::get<3>(this->raw);
                output.emplace_back(scalar_poly(val, poly_factory.zero_tolerance));
            } break;

            // Complex number
            case 4: {
                auto val = std::get<4>(this->raw);
                output.emplace_back(scalar_poly(val, poly_factory.zero_tolerance));
            } break;

            // Real number array
            case 5: {
                const auto& matrix = std::get<5>(this->raw);
                output.reserve(matrix.size());
                for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                    output.emplace_back(scalar_poly(*iter, poly_factory.zero_tolerance));
                }
            } break;

            // Complex number array
            case 6: {
                const auto& matrix = std::get<6>(this->raw);
                output.reserve(matrix.size());
                for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                    output.emplace_back(scalar_poly(*iter, poly_factory.zero_tolerance));
                }
            } break;

        default:
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial array.");
        }

        return output;
    }

    const RawPolynomial AlgebraicOperand::to_raw_polynomial(const MatrixSystem& system) {
        const auto& poly_factory = system.polynomial_factory();
        const auto& context = system.Context();

        switch (this->raw.index()) {
            // Symbol cell input
            case 1: {
                // Error if no data
                const auto& polys = this->raw_symbol_cell_data();
                if (polys.empty()) {
                    throw_error(matlabEngine, errors::bad_param, "Polynomial input array was empty.");
                }
                const auto& first_poly_raw = polys.front();

                Polynomial symbolic_poly = raw_data_to_polynomial(matlabEngine, poly_factory, first_poly_raw);
                return RawPolynomial{symbolic_poly, system.Symbols()}; // Down-convert
            }

            // Operator cell input
            case 2: {
                // Error if no data:
                auto& staging_polys = this->raw_operator_cell_data();
                if (staging_polys.empty()) {
                    throw_error(matlabEngine, errors::internal_error, "Polynomial input array was empty.");
                }

                auto& the_poly = staging_polys.front();
                the_poly.supply_context(context);
                return the_poly.to_raw_polynomial();
            }


            // Real number
            case 3: {
                auto val = std::get<3>(this->raw);
                return scalar_raw_poly(context, val, poly_factory.zero_tolerance);
            }

            // Complex number
            case 4: {
                auto val = std::get<4>(this->raw);
                return scalar_raw_poly(context, val, poly_factory.zero_tolerance);
            }

            // Real number array
            case 5: {
                const auto& matrix = std::get<5>(this->raw);
                if (matrix.size() < 1) {
                    throw_error(matlabEngine, errors::internal_error, "Input array was unexpectedly empty.");
                }
                auto val = matrix.coeff(0, 0);
                return scalar_raw_poly(context, val, poly_factory.zero_tolerance);
            }

            // Complex number array
            case 6: {
                const auto& matrix = std::get<6>(this->raw);
                if (matrix.size() < 1) {
                    throw_error(matlabEngine, errors::internal_error, "Input array was unexpectedly empty.");
                }
                auto val = matrix.coeff(0, 0);
                return scalar_raw_poly(context, val, poly_factory.zero_tolerance);
            }

            default:
                throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a raw polynomial.");
        }
    }

    const std::vector<RawPolynomial> AlgebraicOperand::to_raw_polynomial_array(const MatrixSystem& system)  {
        std::vector<RawPolynomial> output;

        const auto& poly_factory = system.polynomial_factory();
        const auto& context = system.Context();

        switch(this->raw.index()) {
            // Symbol cell input
            case 1: {
                const auto& raw_polys = this->raw_symbol_cell_data();
                const auto& symbols = system.Symbols();
                output.reserve(raw_polys.size());
                for (const auto& raw_poly : raw_polys) {
                   Polynomial symbolic_poly = raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly);
                   output.emplace_back(symbolic_poly, symbols);
                }
            } break;

            // Operator cell input
            case 2: {
                // Error if no data:
                auto& staging_polys = this->raw_operator_cell_data();
                output.reserve(staging_polys.size());
                for (auto& raw_poly : staging_polys) {
                    raw_poly.supply_context(context);
                    output.emplace_back(raw_poly.to_raw_polynomial());
                }
            } break;

            // Real number
            case 3: {
                auto val = std::get<3>(this->raw);
                output.emplace_back(scalar_raw_poly(context, val, poly_factory.zero_tolerance));
            } break;

            // Complex number
            case 4: {
                auto val = std::get<4>(this->raw);
                output.emplace_back(scalar_raw_poly(context, val, poly_factory.zero_tolerance));
            } break;

            // Real number array
            case 5: {
                const auto& matrix = std::get<5>(this->raw);
                output.reserve(matrix.size());
                for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                    output.emplace_back(scalar_raw_poly(context, *iter, poly_factory.zero_tolerance));
                }
            } break;

            // Complex number array
            case 6: {
                const auto& matrix = std::get<6>(this->raw);
                output.reserve(matrix.size());
                for (auto iter = matrix.data(); iter < matrix.data() + matrix.size(); ++iter) {
                    output.emplace_back(scalar_raw_poly(context, *iter, poly_factory.zero_tolerance));
                }
            } break;

        default:
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a raw polynomial.");
        }

        return output;
    }

    std::unique_ptr<ValueMatrix> AlgebraicOperand::to_value_matrix(MatrixSystem& system,
                                                                   std::optional<std::string> desc) {

        const auto& poly_factory = system.polynomial_factory();
        const auto& context = system.Context();
        auto& symbols = system.Symbols();

        switch(this->raw.index()) {
            // Real number
            case 3:
                return std::make_unique<ValueMatrix>(context, symbols, poly_factory.zero_tolerance,
                                                     Eigen::MatrixXd{{std::get<3>(this->raw)}}, desc);

            // Complex number
            case 4:
                return std::make_unique<ValueMatrix>(context, symbols, poly_factory.zero_tolerance,
                                                     Eigen::MatrixXcd{{std::get<4>(this->raw)}}, desc);

            // Real number array
            case 5:
                return std::make_unique<ValueMatrix>(context, symbols, poly_factory.zero_tolerance,
                                                     std::get<5>(this->raw), desc);

            // Complex number array
            case 6:
                return std::make_unique<ValueMatrix>(context, symbols, poly_factory.zero_tolerance,
                                                     std::get<6>(this->raw), desc);
            default:
                throw_error(this->matlabEngine, errors::internal_error,
                            "Only numeric input data can be parsed into a value matrix.");
        }
    }


    std::ostream& operator<<(std::ostream& os, const AlgebraicOperand& operand) {

        auto dim_format = [&os](const std::vector<size_t>& shape) {
            bool once = false;
            for (const auto dim : shape) {
                if (once) {
                    os << " x ";
                }
                os << dim;
                once = true;
            }
        };

        switch (operand.type) {
            default:
            case AlgebraicOperand::InputType::Unknown:
                os << "[Unknown: " << static_cast<int>(operand.type) << "]";
                return os; // done!
            case AlgebraicOperand::InputType::EmptyObject:
                os << "Empty algebraic operand.";
                return os; // done!
            case AlgebraicOperand::InputType::MatrixID:
                os << "Matrix, index " << operand.matrix_key();
                break;
            case AlgebraicOperand::InputType::Monomial:
                os << "Scalar monomial";
                break; //
            case AlgebraicOperand::InputType::Polynomial:
                os << "Scalar polynomial";
                break;
            case AlgebraicOperand::InputType::MonomialArray:
                dim_format(operand.shape);
                os << " monomial";
                break;
            case AlgebraicOperand::InputType::PolynomialArray:
                dim_format(operand.shape);
                os << " polynomial";
                break;
            case AlgebraicOperand::InputType::RealNumber:
                os << "Real scalar";
                break;
            case AlgebraicOperand::InputType::RealNumberArray:
                dim_format(operand.shape);
                os << " real array";
                break;
            case AlgebraicOperand::InputType::ComplexNumber:
                os << "Complex scalar";
                break;
            case AlgebraicOperand::InputType::ComplexNumberArray:
                dim_format(operand.shape);
                os << " complex array";
                break;
        }

        os << " (input as: ";
        switch (operand.format) {
            case AlgebraicOperand::InputFormat::Unknown:
                os << "unknown";
                break;
            case AlgebraicOperand::InputFormat::Integer:
                os << "integer";
                break;
            case AlgebraicOperand::InputFormat::NumericData:
                os << "numeric data";
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
        os << ")";
        return os;
    }


}
