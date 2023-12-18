/**
 * algebraic_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "algebraic_operand.h"

#include "error_codes.h"

#include "matrix_system/matrix_system.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include <sstream>

namespace Moment::mex {
    void AlgebraicOperand::parse_input(matlab::data::Array& input, const bool expect_symbol_cell) {
        // Check type of LHS input
        switch (input.getType()) {
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
                this->parse_as_matrix_key(input);
                return;
            case matlab::data::ArrayType::CELL:
                if (expect_symbol_cell) {
                    this->parse_as_symbol_cell(input);
                } else {
                    this->parse_as_operator_cell(input);
                }
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

    bool AlgebraicOperand::is_scalar() const noexcept {
        switch (this->type) {
            default:
            case InputType::Unknown:
            case InputType::MatrixID:
            case InputType::PolynomialArray:
            case InputType::MonomialArray:
                return false;
            case InputType::Monomial:
            case InputType::Polynomial:
                return true;
        }
    }

    void AlgebraicOperand::parse_as_matrix_key(const matlab::data::Array& raw_input) {
        this->type = InputType::MatrixID;
        this->shape = std::vector<size_t>{0, 0};
        this->raw.emplace<0>(read_as_uint64(matlabEngine, raw_input));
    }

    void AlgebraicOperand::parse_as_symbol_cell(const matlab::data::CellArray& input) {
        this->parse_as_symbolic_polynomial(input);
    }

    void AlgebraicOperand::parse_as_operator_cell(const matlab::data::CellArray& input) {
        if (this->determine_if_polynomial_operator_cell(input)) {
            this->parse_as_operator_polynomial(input);
        } else {
            this->parse_as_symbolic_polynomial(input);
        }
    }

    bool AlgebraicOperand::determine_if_polynomial_operator_cell(const matlab::data::CellArray& cell_input) const {
        // Not polynomial if completely empty
        if (cell_input.isEmpty()) {
            return false;
        }

        // Invalid if first element is not a cell array
        const auto cell_input_iter = cell_input.begin();
        if (cell_input_iter->getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial mode expects symbol cell input.");
        }
        matlab::data::CellArray first_elem_as_cell = *cell_input_iter;

        // If first child element is empty, then it is a length 0 polynomial
        if (first_elem_as_cell.isEmpty()) {
            return true;
        }

        // Otherwise, if first child's child is a cell array, we have a polynomial
        const auto first_child_iter = first_elem_as_cell.begin();
        if (first_child_iter->getType() == matlab::data::ArrayType::CELL) {
            return true;
        }

        // Otherwise, is (probably) a monomial
        return false;
    }

    void AlgebraicOperand::parse_as_operator_monomial(const matlab::data::CellArray& input) {
//        const size_t expected_elements = input.getNumberOfElements();
//        this->type = expected_elements != 1 ? AlgebraicOperand::InputType::MonomialArray
//                                            : AlgebraicOperand::InputType::Monomial;
//        const auto dimensions = input.getDimensions();
//        this->shape.reserve(dimensions.size());
//        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));
//
//        // Prepare object with data
//        this->raw.emplace<2>();
//        auto& raw_vec = this->raw_operator_cell_data();
//        raw_vec.reserve(expected_elements);
//
//        for (const auto& element : input) {
//
//        }
        // TODO: Implement
        throw_error(matlabEngine, errors::internal_error, "Parse as operator monomial not implemented.");

    }

    void AlgebraicOperand::parse_as_operator_polynomial(const matlab::data::CellArray& input) {
        const size_t expected_elements = input.getNumberOfElements();
        this->type = expected_elements != 1 ? AlgebraicOperand::InputType::PolynomialArray
                                            : AlgebraicOperand::InputType::Polynomial;

        const auto dimensions = input.getDimensions();
        this->shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));

        // Prepare object with data
        this->raw.emplace<2>();
        auto& raw_vec = this->raw_operator_cell_data();
        raw_vec.reserve(expected_elements);

        // Each elem is an op-seq polynomial:
        for (const auto& element : input) {
            raw_vec.emplace_back(this->matlabEngine, element, this->name);
        }

    }

    void AlgebraicOperand::parse_as_symbolic_polynomial(const matlab::data::CellArray& cell_input) {
        const size_t expected_elements = cell_input.getNumberOfElements();
        if (expected_elements == 0) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial multiplication expects non-empty operand.");
        }
        this->type = expected_elements != 1 ? AlgebraicOperand::InputType::PolynomialArray
                                            : AlgebraicOperand::InputType::Polynomial;
        const auto dimensions = cell_input.getDimensions();
        this->shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));

        this->raw.emplace<1>();
        auto& raw_vec = this->raw_symbol_cell_data();
        raw_vec.reserve(expected_elements);

        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            raw_vec.emplace_back(read_raw_polynomial_data(matlabEngine, name, *read_iter));
            ++read_iter;
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

    const Polynomial AlgebraicOperand::to_polynomial(const MatrixSystem& system, const bool assume_sorted) {
        const auto& poly_factory = system.polynomial_factory();

        if (this->raw.index() == 1) { // Symbol cell input
            // Error if no data
            const auto& polys = this->raw_symbol_cell_data();
            if (polys.empty()) {
                throw_error(matlabEngine, errors::bad_param, "Polynomial input array was empty.");
            }
            const auto& first_poly_raw =  polys.front();

            // Instantiate raw polynomial
            if (assume_sorted) {
                return raw_data_to_polynomial_assume_sorted(matlabEngine, poly_factory, first_poly_raw);
            }
            return raw_data_to_polynomial(matlabEngine, poly_factory, first_poly_raw);
        } else if (this->raw.index() == 2) { // Operator cell input
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
        } else {
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial.");
        }
    }

    const std::vector<Polynomial>
    AlgebraicOperand::to_polynomial_array(const MatrixSystem& system, bool assume_sorted) {
        const auto& poly_factory = system.polynomial_factory();

        std::vector<Polynomial> output;
        if (this->raw.index() == 1) { // Symbol cell input
            const auto& raw_polys = this->raw_symbol_cell_data();
            output.reserve(raw_polys.size());
            for (const auto& raw_poly : raw_polys) {
                if (assume_sorted) {
                    output.emplace_back(raw_data_to_polynomial_assume_sorted(matlabEngine, poly_factory, raw_poly));
                } else {
                    output.emplace_back(raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly));
                }
            }
        } else if (this->raw.index() == 2) { // Operator cell input
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
        } else {
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial.");
        }

        return output;
    }

    const RawPolynomial AlgebraicOperand::to_raw_polynomial(const MatrixSystem& system) {
        if (this->raw.index() == 1) { // Symbol cell input
            // Error if no data
            const auto& polys = this->raw_symbol_cell_data();
            if (polys.empty()) {
                throw_error(matlabEngine, errors::bad_param, "Polynomial input array was empty.");
            }
            const auto& first_poly_raw = polys.front();

            const auto& poly_factory = system.polynomial_factory();
            Polynomial symbolic_poly = raw_data_to_polynomial(matlabEngine, poly_factory, first_poly_raw);
            return RawPolynomial{symbolic_poly, system.Symbols()}; // Downconvert
        } else if (this->raw.index() == 2) { // Operator cell input
            // Error if no data:
            auto& staging_polys = this->raw_operator_cell_data();
            if (staging_polys.empty()) {
                throw_error(matlabEngine, errors::internal_error, "Polynomial input array was empty.");
            }

            auto& the_poly = staging_polys.front();
            the_poly.supply_context(system.Context());
            return the_poly.to_raw_polynomial();
        } else {
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial.");
        }
    }

    const std::vector<RawPolynomial> AlgebraicOperand::to_raw_polynomial_array(const MatrixSystem& system)  {
        std::vector<RawPolynomial> output;
        if (this->raw.index() == 1) { // Symbol cell input
            const auto& raw_polys = this->raw_symbol_cell_data();
            const auto& poly_factory = system.polynomial_factory();
            const auto& symbols = system.Symbols();
            output.reserve(raw_polys.size());
            for (const auto& raw_poly : raw_polys) {
               Polynomial symbolic_poly = raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly);
               output.emplace_back(symbolic_poly, symbols);
            }
        } else if (this->raw.index() == 2) { // Operator cell input
            // Error if no data:
            auto& staging_polys = this->raw_operator_cell_data();
            output.reserve(staging_polys.size());
            for (auto& raw_poly : staging_polys) {
                raw_poly.supply_context(system.Context());
                output.emplace_back(raw_poly.to_raw_polynomial());
            }
        } else {
            throw_error(matlabEngine, errors::internal_error, "Operand cannot be interpreted as a polynomial.");
        }

        return output;
    }

}
