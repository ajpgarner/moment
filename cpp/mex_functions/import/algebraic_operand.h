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

#include <iosfwd>
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
         * Algebraic input.
         * Can either be a matrix, a symbolic cell (i.e. Polynomial), or an operator cell (i.e. RawPolynomial).
         *
         * Input type is deduced from the format supplied to the operand:
         * Matrix input is:
         *      [uint64, scalar]
         * Symbol cell input is:
         *      { { { [int64, scalar: symbol], [(complex) double: factor], [logical: conjugated], (...) }, ...}
         * Operator cell input is:
         *      { { { [uint64: operator sequence], [(complex) double: factor] }, ...} ...}
         */
        struct AlgebraicOperand {
        public:
            /** Associated MATLAB instance. */
            matlab::engine::MATLABEngine& matlabEngine;

            /** The label of the parameter */
            const std::string name = "Unknown operand";

            /** Before parsing, what format was the input in? */
            enum class InputFormat : unsigned char {
                Unknown,
                Number,
                SymbolCell,
                OperatorCell
            } format = InputFormat::Unknown;

            /** After parsing, what is the operand? */
            enum class InputType : unsigned char {
                Unknown = 0x00,
                EmptyObject = 0x01,
                MatrixID = 0x02,
                Monomial = 0x04,
                MonomialArray = 0x84,
                Polynomial = 0x08,
                PolynomialArray = 0x88
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
             */
            void parse_input(matlab::data::Array& input);

            /** True if operand represents a single scalar object (cf. an array or tensor). */
            [[nodiscard]] bool is_scalar() const noexcept;

            /** True if every element of the operand is a monomial (cf. containing one or more polynomials). */
            [[nodiscard]] bool is_monomial() const noexcept;

            /** True if operand is empty. */
            [[nodiscard]] inline bool is_empty() const noexcept {
                return this->type == InputType::EmptyObject;
            }

            /**
             * If input was a matrix ID, return reference to that matrix.
             */
            [[nodiscard]] const SymbolicMatrix& to_matrix(const MatrixSystem& system) const;

            /**
             * If input can be read as a single polynomial, instantiate it.
             * @param system The matrix system the Polynomial is associated with
             * @param assume_sorted True if input polynomial is in canonical order w.r.t. system polynomial factory.
             */
            [[nodiscard]] const Polynomial to_polynomial(const MatrixSystem& system, bool assume_sorted = false);

            /**
             * If input was an array of polynomials, instantiate them.
             * @param system The matrix system the Polynomial is associated with
             * @param assume_sorted True if input polynomials are in canonical order w.r.t. system polynomial factory.
             */
            [[nodiscard]] const std::vector<Polynomial>
            to_polynomial_array(const MatrixSystem& system, bool assume_sorted = false);

            [[nodiscard]] const RawPolynomial to_raw_polynomial(const MatrixSystem& system);

            [[nodiscard]] const std::vector<RawPolynomial> to_raw_polynomial_array(const MatrixSystem& system);

        private:
            void parse_as_matrix_key(const matlab::data::Array& input);

            void parse_cell(const matlab::data::Array& input);

            [[nodiscard]] InputFormat
            infer_format_from_scalar_object(const matlab::data::CellArray& input, size_t outer_index) const;

            [[nodiscard]] InputType
            infer_type_from_valid_cell(const matlab::data::CellArray& input) const;

            void parse_as_operator_cell(const matlab::data::CellArray& input);

            void parse_as_symbol_cell(const matlab::data::CellArray& input);

        public:
            /** Debug info on operand */
            friend std::ostream& operator<<(std::ostream&, const AlgebraicOperand& operand);
        };
    }
}