/**
 * moment_simplifier_no_wrapping.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_simplifier.h"

#include <tuple>
#include <utility>


namespace Moment::Pauli {
    class PauliContext;

    class MomentSimplifierNoWrappingChain : public MomentSimplifier {
    public:
        constexpr const static uint64_t expected_label = 0x8000000000000000;

        const size_t qubits;

    public:
        explicit MomentSimplifierNoWrappingChain(const PauliContext& context);

        [[nodiscard]] sequence_storage_t canonical_sequence(const std::span<const oper_name_t> input) const final;

        [[nodiscard]] bool is_canonical(const std::span<const oper_name_t> input) const noexcept final {
            return (input.empty() ? true : (input[0] <= 2));
        }

        /** Gets the smallest qubit in a sequence, or 0 if sequence is empty. */
        [[nodiscard]] static inline constexpr size_t chain_minimum(const std::span<const oper_name_t> input) {
            return (input.empty() ? 0 : input[0] / 3);
        }

        /** Gets the smallest qubit in a sequence, or qubits if sequence is empty. */
        [[nodiscard]] inline size_t chain_maximum(const std::span<const oper_name_t> input) {
            return (input.empty() ? qubits : input.back() / 3);
        }
    };

    class MomentSimplifierNoWrappingLattice : public MomentSimplifier {

    public:
        constexpr const static uint64_t expected_label = 0xc000000000000000;

        const size_t qubits;

        const size_t column_height;

        const size_t row_width;

        /** Number of operators defining one column (3 * column height) */
        const size_t column_op_height;

    public:
        explicit MomentSimplifierNoWrappingLattice(const PauliContext& context);

        [[nodiscard]] sequence_storage_t canonical_sequence(const std::span<const oper_name_t> input) const final;

        [[nodiscard]] bool is_canonical(const std::span<const oper_name_t> input) const noexcept final;

        /** Gets the smallest (row, col) in a sequence, or (0,0) if sequence is empty. */
        [[nodiscard]] std::pair<size_t,size_t> lattice_minimum(const std::span<const oper_name_t> input) const;

        /**
         * Gets the largest (row, col) in a sequence of operators, or (rows, cols) if sequence is empty.
         * @param input A span over operators.
         */
        [[nodiscard]] std::pair<size_t, size_t> lattice_maximum(const std::span<const oper_name_t> input) const;

        /**
         * Gets the largest (row, col) in a sequence of site indices, or (rows, cols) if sequence is empty.
         * @param input A span over qubit indices.
         */
        [[nodiscard]] std::pair<size_t, size_t> lattice_maximum(const std::span<const size_t> input) const;


    };
}