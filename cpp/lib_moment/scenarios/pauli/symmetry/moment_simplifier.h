/**
 * moment_simplifier.h
 *
 * Abstract base class for Pauli moment simplification.
 * For implementations of this class, see nonwrapping_simplifier.cpp/.h and site_hasher.h.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"
#include "hashed_sequence.h"

#include "dictionary/operator_sequence.h"

#include "../pauli_context.h"

#include <cassert>

#include <memory>
#include <span>

namespace Moment::Pauli {
    class PauliContext;

    /**
     * Interface for site hasher
     */
    class MomentSimplifier {
    public:
        /** Helper tag for polymorphism. */
        const uint64_t impl_label;

    public:
        /** Attached context */
        const PauliContext& context;

    protected:
        /**
         * Construct a site-hasher with information known already
         * @param context The Pauli context defining the chain or lattice
         * @param impl_label Helper tag for polymorphism.
         */
        explicit MomentSimplifier(const PauliContext& context, uint64_t impl_label);

    public:
        /**
         * Virtual destructor for abstract base class.
         */
        virtual ~MomentSimplifier() noexcept = default;

        /**
         * Return a canonical representative of the equivalence class a string of operators is in.
         * @param input A view to operator sequence data.
         * @return A representative of the equivalence class the operator sequence is in, as a raw vector of operators.
         */
        [[nodiscard]] virtual sequence_storage_t
        canonical_sequence(const std::span<const oper_name_t> input) const = 0;

        /**
         * Return a canonical representative of the equivalence class an operator sequence is in
         * @param input A view to operator sequence data.
         * @return A representative of the equivalence class the operator sequence is in, as an OperatorSequence.
         */
        [[nodiscard]] OperatorSequence canonical_sequence(const OperatorSequence& input) const;

        /**
         * Return a canonical representative of the equivalence class an operator sequence is in (alias).
         * @param input A view to operator sequence data.
         * @return A representative of the equivalence class the operator sequence is in, as an OperatorSequence.
         */
        [[nodiscard]] inline OperatorSequence operator()(const OperatorSequence& input) const {
            return this->canonical_sequence(input);
        }

        /**
         * Test if a sequence is canonical or not
         * @param input A view to operator sequence data.
         */
        [[nodiscard]] virtual bool is_canonical(const std::span<const oper_name_t> input) const noexcept = 0;

        /**
         * Return an instantiation of a site hasher.
         * @param qubit_count The maximum number of qubits in the hasher.
         * @param col_size The number of qubits in a column.
         * @return
         */
        [[nodiscard]] static std::unique_ptr<MomentSimplifier> make(const PauliContext& context);

        /**
         * Create a copied list of operators, offset as if it were a chain.
         * If offset is invalid (e.g. it pushes sequence out of chain in non-wrapping mode), behaviour is undefined.
         */
        [[nodiscard]] virtual sequence_storage_t chain_offset(const std::span<const oper_name_t> input,
                                                              ptrdiff_t offset) const = 0;

        /**
         * Create a copied OperatorSequence, offset as if it were a chain.
         * If offset is invalid (e.g. it pushes sequence out of chain in non-wrapping mode), behaviour is undefined.
         */
        [[nodiscard]] inline OperatorSequence
        chain_offset(const OperatorSequence& input, const ptrdiff_t offset) const {
            return OperatorSequence{OperatorSequence::ConstructPresortedFlag{},
                                    this->chain_offset(input.raw(), offset), this->context, input.get_sign()};
        }

        /**
         * Create a copied list of operators, offset as if it were in a lattice.
         * If offset is invalid (e.g. it pushes sequence out of lattice in non-wrapping mode), behaviour is undefined.
         */
        [[nodiscard]] virtual sequence_storage_t lattice_offset(const std::span<const oper_name_t> input,
                                                                ptrdiff_t row_offset,
                                                                ptrdiff_t col_offset) const = 0;

        /**
         * Create a copied OperatorSequence, offset as if it were in a lattice.
         * If offset is invalid (e.g. it pushes sequence out of lattice in non-wrapping mode), behaviour is undefined.
         */
        [[nodiscard]] inline OperatorSequence lattice_offset(const OperatorSequence& input,
                                                             const ptrdiff_t row_offset,
                                                             const ptrdiff_t col_offset) const {
            return OperatorSequence{OperatorSequence::ConstructPresortedFlag{},
                                    this->lattice_offset(input.raw(), row_offset, col_offset),
                                    this->context, input.get_sign()};
        }

    };
}