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
    class OperatorSequenceGenerator;

    namespace errors {
        /**
         * Error thrown if context cannot be created for some reason.
         */
        class bad_pauli_context : public std::runtime_error {
        public:
            explicit bad_pauli_context(const std::string& what) : std::runtime_error{what} { }
        };
    }

    namespace Pauli {
        class MomentSimplifier;
        struct NearestNeighbourIndex;
        class PauliDictionary;
        class PauliSequenceGenerator;

        /**
          * Does the system wrap or tile?
          * In 1D, this means qubit N-1 neighbours qubit 0.
          * In 2D, this means the right  column neighbours the left column, and the top row neighbours the bottom.
          */
         enum class WrapType : bool {
            /** Qubits 0 and N-1 are not neighbours. */
            None = false,
            /** Qubits in rows (resp. cols) 0 and N-1 are neighbours. */
            Wrap = true
        };

        /**
         * Does the system have translational symmetry?
         */
        enum class SymmetryType : bool {
            /** No translational symmetry. */
            None = false,
            /** Lattice invariance symmetry if wrap enabled; 'thermodynamic limit' if wrap disabled. */
            Translational = true
        };


        /**
         * Context for spin system.
         */
        class PauliContext : public Context {
        public:
            /**
            * The total number of qubits defined in context.
            */
            const size_t qubit_size;

            /**
             * If a 2D spin lattice, the number of qubits in one column.
             * Adopting a COLUMN MAJOR scheme, this is the stride of the major index.
             * If zero, then context is set to chain mode for purposes of neighbours.
             */
            const size_t col_height;

            /**
             * If a 2D spin lattice, the number of qubits in one row.
             * Adopting a COLUMN MAJOR scheme, this is the number of major elements
             * In 2D mode, indexing is COLUMN MAJOR: qubit number = col * col_height + row.
             */
            const size_t row_width;

            /**
             * Does the system wrap/tile?
             */
             const WrapType wrap;

            /**
             * Does the system have translational symmetry?
             */
            const enum class SymmetryType translational_symmetry;

        private:
             PauliDictionary* dictionary_ptr = nullptr;

             /** Hasher, for calculating translational symmetry equivalence classes */
             std::unique_ptr<MomentSimplifier> tx_hasher;

        public:
            /**
             * Construct a context for a chain of qubits.
             * @param qubits The number of unique qubits in the scenario.
             * @param wrap True to make the system wrap (or tile in 2D), in terms of neighbouring qubits.
             * @param lattice_col_height The number of qubits in one column, of 2D lattice, or set to 0 for a 1D chain.
             * @throws bad_pauli_context If the lattice row size is not valid (0, or divisor of qubits).
             */
            explicit PauliContext(size_t qubits, WrapType wrap = WrapType::None,
                                  SymmetryType translational_symmetry = SymmetryType::None);

            /**
             * Construct a context for a lattice of qubits.
             * @param qubits The number of unique qubits in the scenario.
             * @param wrap True to make the system wrap (or tile in 2D), in terms of neighbouring qubits.
             * @param col_height The number of qubits in one column (i.e. number of rows in lattice).
             * @param row_width The number of qubits in one row (i.e. number of columns in lattice).
             * @throws bad_pauli_context If the lattice row size is not valid (e.g. because col_height is 0)
             */
            explicit PauliContext(size_t col_height, size_t row_width, WrapType wrap = WrapType::None,
                                  SymmetryType translational_symmetry = SymmetryType::None);

            ~PauliContext() noexcept;

            inline const PauliDictionary& pauli_dictionary() const noexcept {
                assert (dictionary_ptr);
                return *dictionary_ptr;
            }

            [[nodiscard]] bool can_be_nonhermitian() const noexcept final {
                return false;
            }

            [[nodiscard]] inline bool is_lattice() const noexcept {
                return this->col_height != 0;
            }

            [[nodiscard]] bool can_have_aliases() const noexcept final {
                // If we have symmetry, then things like <X3> -> <X1>, <X2Y5> -> <X1Y4>, etc.
                return this->translational_symmetry == SymmetryType::Translational;
            }

            [[nodiscard]] OperatorSequence simplify_as_moment(OperatorSequence&& seq) const final;

            [[nodiscard]] bool can_be_simplified_as_moment(const OperatorSequence& seq) const final;

            bool additional_simplification(sequence_storage_t &op_sequence, SequenceSignType &sign) const final;

            [[nodiscard]] OperatorSequence
            multiply(const OperatorSequence &lhs, const OperatorSequence &rhs) const final;

            /**
             * 1/2 [lhs, rhs] = (lhs * rhs - rhs * lhs)
             * Result might be zero.
             */
            [[nodiscard]] OperatorSequence commutator(const OperatorSequence &lhs, const OperatorSequence &rhs) const;

            /**
             * 1/2 {lhs, rhs} = 1/2 * (lhs * rhs + rhs * lhs)
             * Result might be zero.
             */
            [[nodiscard]] OperatorSequence anticommutator(const OperatorSequence &lhs, const OperatorSequence &rhs) const;


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

            /**
             * Pauli sigma X operator at site (row, col).
             * Note column major storage order.
             */
            [[nodiscard]] inline OperatorSequence sigmaX(const oper_name_t row, const oper_name_t col,
                                                  SequenceSignType sign = SequenceSignType::Positive) const {
                return sigmaX(col*this->col_height + row, sign);
            }

            /** Pauli sigma Y operator at site N. */
            [[nodiscard]] OperatorSequence sigmaY(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /**
             * Pauli sigma Y operator at site (row, col).
             * Note column major storage order.
             */
            [[nodiscard]] inline OperatorSequence sigmaY(const oper_name_t row, const oper_name_t col,
                                                         SequenceSignType sign = SequenceSignType::Positive) const {
                return sigmaY(col*this->col_height + row, sign);
            }

            /** Pauli sigma Z operator at site N. */
            [[nodiscard]] OperatorSequence sigmaZ(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /**
             * Pauli sigma Z operator at site (row, col).
             * Note column major storage order.
             */
            [[nodiscard]] inline OperatorSequence sigmaZ(const oper_name_t row, const oper_name_t col,
                                                         SequenceSignType sign = SequenceSignType::Positive) const {
                return sigmaZ(col*this->col_height + row, sign);
            }

            /**
             * Simplifier object.
             */
            [[nodiscard]] const MomentSimplifier& moment_simplifier() const;

            /**
             * Convert qubit offset to [row, col] pair.
             * Undefined behaviour if context is not a lattice.
             */
             [[nodiscard]] inline std::pair<size_t, size_t>
             qubit_offset_to_indices(const size_t offset) const noexcept {
                 return {offset % this->col_height, offset / this->col_height};
             }

             /**
              * Convert row, column pair to qubit offset.
              * Undefined behaviour if context is not a lattice.
              */
             [[nodiscard]] inline size_t qubit_indices_to_offset(const size_t row, const size_t col) const noexcept {
                 return (col * this->col_height) + row;
             }

        protected:
            [[nodiscard]] std::unique_ptr<OperatorSequenceGenerator> new_osg(size_t word_length) const override;


        };


    }}