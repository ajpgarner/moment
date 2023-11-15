/**
 * algebraic_operand.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "read_polynomial.h"

#include "symbolic/polynomial.h"

#include "MatlabDataArray.hpp"

#include <variant>
#include <vector>


namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class MatrixSystem;
    class SymbolicMatrix;

    namespace mex {

        /**
         * Algebraic input
         */
        struct AlgebraicOperand {
        public:
            enum class InputType {
                Unknown,
                MatrixID,
                Polynomial,
                PolynomialArray
            } type = InputType::Unknown;

            std::vector<size_t> shape;
            std::variant<size_t, std::vector<std::vector<raw_sc_data>>> raw;

        public:

            [[nodiscard]] constexpr size_t matrix_key() const { return std::get<0>(this->raw); }

            [[nodiscard]] constexpr std::vector<std::vector<raw_sc_data>>& raw_polynomials() {
                return std::get<1>(this->raw);
            }

            [[nodiscard]] constexpr const std::vector<std::vector<raw_sc_data>>& raw_polynomials() const {
                return std::get<1>(this->raw);
            }

        public:
            AlgebraicOperand() = default;

            AlgebraicOperand(const AlgebraicOperand& lhs) = delete;

            AlgebraicOperand(AlgebraicOperand&& lhs) = default;

            AlgebraicOperand& operator=(const AlgebraicOperand& rhs) = delete;

            AlgebraicOperand& operator=(AlgebraicOperand&& rhs) = default;

            [[nodiscard]] bool is_scalar() const;

            void parse_input(matlab::engine::MATLABEngine& engine, const std::string& name, matlab::data::Array& input);

            void parse_as_matrix_key(matlab::engine::MATLABEngine& engine,
                                     const std::string& name, matlab::data::Array& input);

            void parse_as_polynomial(matlab::engine::MATLABEngine& engine,
                                     const std::string& name, matlab::data::Array& input);

            [[nodiscard]] const SymbolicMatrix& to_matrix(matlab::engine::MATLABEngine& engine,
                                                                const MatrixSystem& system) const;

            [[nodiscard]] const Polynomial to_polynomial(matlab::engine::MATLABEngine& matlabEngine,
                                                         const MatrixSystem& system,
                                                         bool assume_sorted = false) const;

            [[nodiscard]] const std::vector<Polynomial>
            to_polynomial_array(matlab::engine::MATLABEngine& matlabEngine, const MatrixSystem& system,
                                bool assume_sorted = false) const;

        };
    }
}