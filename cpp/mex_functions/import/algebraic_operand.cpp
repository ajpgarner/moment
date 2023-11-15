/**
 * algebraic_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "algebraic_operand.h"

#include "matrix_system/matrix_system.h"

#include "error_codes.h"
#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include <sstream>

namespace Moment::mex {
    void AlgebraicOperand::parse_input(matlab::engine::MATLABEngine& engine, const std::string& name,
                                       matlab::data::Array& input) {
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
                this->parse_as_matrix_key(engine, name, input);
                break;
            case matlab::data::ArrayType::CELL:
                this->parse_as_polynomial(engine, name, input);
                break;
            default:
            case matlab::data::ArrayType::UNKNOWN: {
                std::stringstream ss;
                ss << name << " was not a valid operand.";
                throw_error(engine, errors::bad_param, ss.str());
            }
            break;
        }
    }

    bool AlgebraicOperand::is_scalar() const {
        switch (this->type) {
            default:
            case InputType::Unknown:
            case InputType::MatrixID:
            case InputType::PolynomialArray:
                return false;
            case InputType::Polynomial:
                return true;
        }
    }

    void AlgebraicOperand::parse_as_matrix_key(matlab::engine::MATLABEngine& matlabEngine, const std::string& name,
                                               matlab::data::Array& raw_input) {
        this->type = InputType::MatrixID;
        this->shape = std::vector<size_t>{0, 0};
        this->raw.emplace<0>(read_as_uint64(matlabEngine, raw_input));
    }

    void AlgebraicOperand::parse_as_polynomial(matlab::engine::MATLABEngine& matlabEngine, const std::string& name,
                                               matlab::data::Array& raw_input) {
        const size_t expected_elements = raw_input.getNumberOfElements();
        if (expected_elements == 0) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial multiplication expects non-empty operand.");
        }
        this->type = expected_elements != 1 ? AlgebraicOperand::InputType::PolynomialArray
                                            : AlgebraicOperand::InputType::Polynomial;
        const auto dimensions = raw_input.getDimensions();
        this->shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(this->shape));

        if (raw_input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial mode expects symbol cell input.");
        }

        this->raw.emplace<1>();
        auto& raw_vec = this->raw_polynomials();
        raw_vec.reserve(expected_elements);

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = raw_input;
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            raw_vec.emplace_back(read_raw_polynomial_data(matlabEngine, name, *read_iter));
            ++read_iter;
        }
    }

    const SymbolicMatrix& AlgebraicOperand::to_matrix(matlab::engine::MATLABEngine& matlabEngine,
                                                            const MatrixSystem& matrixSystem) const {
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

    const Polynomial
    AlgebraicOperand::to_polynomial(matlab::engine::MATLABEngine& matlabEngine, const MatrixSystem& system,
                                    bool assume_sorted) const {
        if ((this->type != InputType::Polynomial) && (this->type != InputType::PolynomialArray)) {
            throw_error(matlabEngine, errors::internal_error, "Operand was not a polynomial.");
        }
        const auto& polys = this->raw_polynomials();
        if (polys.empty()) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial input array was empty.");
        }
        auto& first_poly_raw =  polys.front();
        if (assume_sorted) {
            return raw_data_to_polynomial_assume_sorted(matlabEngine, system.polynomial_factory(), first_poly_raw);
        }
        return raw_data_to_polynomial(matlabEngine, system.polynomial_factory(), first_poly_raw);
    }

    const std::vector<Polynomial>
    AlgebraicOperand::to_polynomial_array(matlab::engine::MATLABEngine& matlabEngine,
                                          const MatrixSystem& system, bool assume_sorted) const {
        if ((this->type != InputType::Polynomial) && (this->type != InputType::PolynomialArray)) {
            throw_error(matlabEngine, errors::internal_error, "Operand was not a polynomial.");
        }
        const auto& poly_factory = system.polynomial_factory();
        const auto& raw_polys = this->raw_polynomials();
        std::vector<Polynomial> output;
        output.reserve(raw_polys.size());
        if (assume_sorted) {
            for (const auto& raw_poly: raw_polys) {
                output.emplace_back(raw_data_to_polynomial_assume_sorted(matlabEngine, poly_factory, raw_poly));
            }
        } else {
            for (const auto& raw_poly: raw_polys) {
                output.emplace_back(raw_data_to_polynomial(matlabEngine, poly_factory, raw_poly));
            }
        }
        return output;
    }

}