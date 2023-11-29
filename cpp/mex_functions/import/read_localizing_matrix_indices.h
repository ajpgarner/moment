/**
 * read_localizing_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/raw_polynomial.h"

#include "matrix_system/localizing_matrix_index.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"
#include "scenarios/pauli/pauli_localizing_matrix_indices.h"
#include "scenarios/pauli/pauli_polynomial_lm_indices.h"

#include "read_polynomial.h"

#include <cassert>

#include <memory>
#include <variant>
#include <vector>

namespace Moment {

    class MatrixSystem;
    namespace mex {

        class StagingPolynomial;


        class LocalizingMatrixIndexImporter {
        public:
            enum class ExpressionType {
                /** Unknown */
                Unknown,
                /** Monomial, defined by operator sequence. */
                OperatorSequence,
                /** Polynomial, defined by symbol cell. */
                SymbolCell,
                /** Polynomial, defined by operators. */
                OperatorCell
            };


            using raw_word_storage_t = std::variant<std::vector<oper_name_t>,
                    std::vector<raw_sc_data>,
                    std::unique_ptr<StagingPolynomial>>;

        private:
            matlab::engine::MATLABEngine& matlabEngine;

            /** NPA Hierarchy level */
            size_t hierarchy_level = 0;

            /** Restrict to nearest neighbours ? */
            size_t nearest_neighbour = 0;

            /** Add one to operator IDs */
            bool matlab_indexing = true;

            /** Raw type */
            ExpressionType expression_type = ExpressionType::Unknown;

            /** Word for localizing matrix*/
            raw_word_storage_t localizing_expression;

            /** Linked matrix system */
            MatrixSystem* matrix_system = nullptr;


        public:
            explicit LocalizingMatrixIndexImporter(matlab::engine::MATLABEngine& engine) : matlabEngine{engine} { }

            inline void set_matlab_indexing(bool enabled) noexcept {
                this->matlab_indexing = enabled;
            }

            [[nodiscard]] inline ExpressionType get_expression_type() const noexcept {
                return this->expression_type;
            }

            [[nodiscard]] inline bool has_nn_info() const noexcept {
                return this->nearest_neighbour != 0;
            }

            size_t read_level(const matlab::data::Array& expr);

            size_t read_nearest_neighbour(const matlab::data::Array& expr);

            void read_localizing_expression(const matlab::data::Array& expr, ExpressionType expr_type);

            inline void link_matrix_system(MatrixSystem* system) {
                assert(this->matrix_system == nullptr);
                this->matrix_system = system;
            }

            /**
             * Partially stage operator-cell polynomial for raw index creation.
             * @rlock Read lock to matrix system on which operators are defined.
             */
            void supply_context_only(const MaintainsMutex::ReadLock& rlock);

            /**
             * Partially stage operator-cell polynomial, and return if all resulting symbols are known.
             * @rlock Read lock to matrix system where symbols are defined.
             */
            [[nodiscard]] bool attempt_to_find_symbols_from_op_cell(const MaintainsMutex::ReadLock& rlock);

            /**
             * Finishing staging operator-cell polynomial, in case where some symbols were missing.
             * @rlock Write lock to matrix system where symbols are defined.
             */
            void register_symbols_in_op_cell(const MaintainsMutex::WriteLock& wlock);

            [[nodiscard]] LocalizingMatrixIndex to_monomial_index() const;

            [[nodiscard]] PolynomialLMIndex to_polynomial_index() const;

            [[nodiscard]] Pauli::PauliLocalizingMatrixIndex to_pauli_monomial_index() const;

            [[nodiscard]] Pauli::PauliPolynomialLMIndex to_pauli_polynomial_index() const;

            [[nodiscard]] std::pair<size_t, RawPolynomial> to_raw_polynomial_index() const;

            [[nodiscard]] std::pair<Pauli::NearestNeighbourIndex, RawPolynomial> to_pauli_raw_polynomial_index() const;


        };

    }
}