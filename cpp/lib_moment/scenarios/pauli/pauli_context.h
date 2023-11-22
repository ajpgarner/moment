/**
 * pauli_context.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../context.h"

#include <iosfwd>
#include <stdexcept>

namespace Moment {
    class ContextualOS;

    namespace errors {
        /**
         * Error thrown if context cannot be created for some reason.
         */
        class bad_pauli_context : std::runtime_error {
        public:
            bad_pauli_context(const std::string& what) : std::runtime_error{what} { }
        };
    }

    namespace Pauli {

        class NearestNeighbourIndex;
        class PauliSequenceGenerator;

        class PauliContext : public Context {
        public:
            /**
            * The total number of qubits defined.
            */
            const oper_name_t qubit_size;

            /**
             * If a 2D spin lattice instead of a 1D spin chain, the number of qubits in one row.
             */
            const oper_name_t row_width = 0;

            /**
             * If a 2D spin lattice, the number of qubits in a column.
             * Either row_width * col_width = qubit_size, or both row_width and col_width are 0.
             */
            const oper_name_t col_width = 0;

            /**
             * Does the system wrap or tile?
             * In 1D, this means qubit N-1 neighbours qubit 0.
             * In 2D, this means the right  column neighbours the left column, and the top row neighbours the bottom.
             */
             const bool wrap = false;

        public:
            /**
             * Construct a context for Pauli matrices
             * @param qubits The number of unique qubits in the scenario.
             * @param wrap True to make the system wrap (or tile in 2D), in terms of neighbouring qubits.
             * @param lattice_row_size The number of qubits in one row of a 2D lattice, or 0 for a 1D chain.
             * @throws bad_pauli_context If the lattice row size is not valid (0, or divisor of qubits).
             */
            explicit PauliContext(oper_name_t qubits, bool wrap = false, oper_name_t lattice_row_size = 0);

            [[nodiscard]] bool can_be_nonhermitian() const noexcept final {
                return false;
            }

            [[nodiscard]] inline bool is_lattice() const noexcept {
                return this->row_width != 0;
            }

            bool additional_simplification(sequence_storage_t &op_sequence, SequenceSignType &sign) const final;

            void multiply(OperatorSequence &lhs, const OperatorSequence &rhs) const final;

            [[nodiscard]] OperatorSequence conjugate(const OperatorSequence &seq) const final;

            void format_sequence(ContextualOS &os, const OperatorSequence &seq) const final;

            void format_raw_sequence(ContextualOS &os, const sequence_storage_t &seq) const final;

            [[nodiscard]] std::string to_string() const final;

            using Context::operator_sequence_generator;
            [[nodiscard]] const OperatorSequenceGenerator&
            operator_sequence_generator(const NearestNeighbourIndex& index, bool conjugated=false) const;

            /** Pauli sigma X  operator at site N. */
            [[nodiscard]] OperatorSequence sigmaX(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /** Pauli sigma Y operator at site N. */
            [[nodiscard]] OperatorSequence sigmaY(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /** Pauli sigma Z operator at site N. */
            [[nodiscard]] OperatorSequence sigmaZ(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

        protected:
            [[nodiscard]] std::unique_ptr<OperatorSequenceGenerator> new_osg(size_t word_length) const override;
        };


    }}