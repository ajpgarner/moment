/**
 * pauli_context.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../context.h"
#include <iosfwd>

namespace Moment {
    class ContextualOS;

    namespace Pauli {

        class PauliContext : public Context {
        public:
            /**
            * The total number of qubits defined.
            */
            const oper_name_t qubit_size;

            /**
            * The maximum distance between points to consider in the moment matrix
            */
            const oper_name_t moment_matrix_range = 1;

        public:
            explicit PauliContext(oper_name_t qubits, oper_name_t range = 1) noexcept;

        public:
            [[nodiscard]] bool can_be_nonhermitian() const noexcept final {
                return false;
            }

            bool additional_simplification(sequence_storage_t &op_sequence, SequenceSignType &sign) const final;

            void multiply(OperatorSequence &lhs, const OperatorSequence &rhs) const final;

            [[nodiscard]] OperatorSequence conjugate(const OperatorSequence &seq) const final;

            void format_sequence(ContextualOS &os, const OperatorSequence &seq) const final;

            void format_raw_sequence(ContextualOS &os, const sequence_storage_t &seq) const final;

            /** Pauli sigma X  operator at site N. */
            [[nodiscard]] OperatorSequence sigmaX(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /** Pauli sigma Y operator at site N. */
            [[nodiscard]] OperatorSequence sigmaY(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;

            /** Pauli sigma Z operator at site N. */
            [[nodiscard]] OperatorSequence sigmaZ(const oper_name_t qubit,
                                                  SequenceSignType sign = SequenceSignType::Positive) const;
        };


    }}