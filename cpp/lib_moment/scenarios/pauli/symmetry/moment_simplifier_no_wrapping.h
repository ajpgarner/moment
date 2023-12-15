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

namespace Moment {
    class RawPolynomial;
    class OperatorSequence;

    namespace Pauli {
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

            /**
             * Gets one beyond the highest-indexed non-trivial qubit in a chain, or 0 if sequence is empty.
             * Useful for determining an 'effective size' of a chain.
             * @range From 0 to qubits inclusive.
             */
            [[nodiscard]] static inline size_t chain_supremum(const std::span<const oper_name_t> input) noexcept {
                return (input.empty() ? 0 : ((input.back() / 3)+1));
            }

            /** Gets the largest non-trivial qubit in a raw polynomial, or zero if polynomial is empty. */
            [[nodiscard]] static size_t chain_supremum(const RawPolynomial& input) noexcept;


            using MomentSimplifier::chain_offset;
            using MomentSimplifier::lattice_offset;

            [[nodiscard]] sequence_storage_t chain_offset(const std::span<const oper_name_t> input,
                                                          ptrdiff_t offset) const final;

            [[nodiscard]] sequence_storage_t lattice_offset(const std::span<const oper_name_t> input,
                                                            ptrdiff_t col_offset, ptrdiff_t row_offset) const final;


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

            /**
             * Gets the smallest (row, col) in a sequence, or (0,0) if sequence is empty.
             */
            [[nodiscard]] std::pair<size_t,size_t> lattice_minimum(const std::span<const oper_name_t> input) const;

            /**
             * Gets one larger than (row, col) in a sequence of operators, or (0, 0) if sequence is empty.
             * Essentially is an 'effective size'.
             * @param input A span over operators.
             * @return first: effective col height (max row), second: effective row width (max col).
             */
            [[nodiscard]] std::pair<size_t, size_t> lattice_supremum(const std::span<const oper_name_t> input) const;

            /**
             * Gets one larger than (row, col) in a sequence of site indices, or (0, 0) if sequence is empty.
             * Essentially is an 'effective size'.
             * @param input A span over qubit indices.
             * @return first: effective col height (max row), second: effective row width (max col).
             */
            [[nodiscard]] std::pair<size_t, size_t> lattice_supremum(const std::span<const size_t> input) const;

            /**
            * Gets one larger than (row, col) from a raw polynomial (0, 0) if polynomial is empty.
            * Essentially is an 'effective size'.
            * @param input A raw polynomial.
            * @return first: effective col height (max row), second: effective row width (max col).
            */
            [[nodiscard]] std::pair<size_t, size_t> lattice_supremum(const RawPolynomial& input) const;

            using MomentSimplifier::chain_offset;
            using MomentSimplifier::lattice_offset;

            [[nodiscard]] sequence_storage_t chain_offset(const std::span<const oper_name_t> input,
                                                          ptrdiff_t offset) const final;

            [[nodiscard]] sequence_storage_t lattice_offset(const std::span<const oper_name_t> input,
                                                            ptrdiff_t row_offset, ptrdiff_t col_offset) const final;


        };

    }}