/**
 * algebraic_operand.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "read_polynomial.h"
#include "read_opseq_polynomial.h"

#include "dictionary/raw_polynomial.h"
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
            matlab::engine::MATLABEngine& matlabEngine;

            /** The label of the parameter */
            const std::string name = "Unknown operand";

            /** What sort of input was the operand? */
            enum class InputType {
                Unknown,
                EmptyObject,
                MatrixID,
                Monomial,
                MonomialArray,
                Polynomial,
                PolynomialArray
            } type = InputType::Unknown;

            /** Dimensions of the object */
            std::vector<size_t> shape;

        protected:
            /** The actual data (union) */
            std::variant<size_t, std::vector<std::vector<raw_sc_data>>, std::vector<StagingPolynomial>> raw;

        public:

            [[nodiscard]] inline size_t matrix_key() const { return std::get<0>(this->raw); }

            [[nodiscard]] inline std::vector<std::vector<raw_sc_data>>& raw_symbol_cell_data() {
                return std::get<1>(this->raw);
            }

            [[nodiscard]] inline const std::vector<std::vector<raw_sc_data>>& raw_symbol_cell_data() const {
                return std::get<1>(this->raw);
            }

            [[nodiscard]] inline std::vector<StagingPolynomial>& raw_operator_cell_data() {
                return std::get<2>(this->raw);
            }

            [[nodiscard]] inline const std::vector<StagingPolynomial>& raw_operator_cell_data() const {
                return std::get<2>(this->raw);
            }

        public:
            AlgebraicOperand(matlab::engine::MATLABEngine& engine, const std::string& name_in)
                : matlabEngine{engine}, name{std::move(name_in)} { }

            AlgebraicOperand(const AlgebraicOperand& lhs) = delete;

            AlgebraicOperand(AlgebraicOperand&& lhs) = default;

            /**
             * Read raw input.
             * @param input The input data to parse.
             * @param expecting_symbols True if cell arrays are expected to be symbol cell (false for operator cell).
             */
            void parse_input(matlab::data::Array& input, bool expecting_symbols = false);

            /** True if operand represents a single scalar object (cf. an array or tensor). */
            [[nodiscard]] bool is_scalar() const noexcept;

            /** True if operand is empty. */
            [[nodiscard]] inline bool is_empty() const noexcept {
                return this->type == InputType::EmptyObject;
            }

            /**
             * If input was a matrix ID, return reference to that matrix.
             */
            [[nodiscard]] const SymbolicMatrix& to_matrix(const MatrixSystem& system) const;


            [[nodiscard]] const Polynomial to_polynomial(const MatrixSystem& system, bool assume_sorted = false);

            [[nodiscard]] const std::vector<Polynomial>
            to_polynomial_array(const MatrixSystem& system, bool assume_sorted = false);

            [[nodiscard]] const RawPolynomial to_raw_polynomial(const MatrixSystem& system);

            [[nodiscard]] const std::vector<RawPolynomial> to_raw_polynomial_array(const MatrixSystem& system);

        private:
            void parse_as_matrix_key(const matlab::data::Array& input);

            void parse_as_symbol_cell(const matlab::data::CellArray& input);

            void parse_as_operator_cell(const matlab::data::CellArray& input);

            /** True if polynomial, false if monomial */
            [[nodiscard]] bool determine_if_polynomial_operator_cell(const matlab::data::CellArray& input) const;

            void parse_as_operator_monomial(const matlab::data::CellArray& input);

            void parse_as_operator_polynomial(const matlab::data::CellArray& input);

            void parse_as_symbolic_polynomial(const matlab::data::CellArray& input);

        };
    }
}