/**
 * site_hasher.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"
#include "hashed_sequence.h"

#include <cassert>

#include <span>

namespace Moment {
    class OperatorSequence;
}

namespace Moment::Pauli {
    class PauliContext;

    /**
     * Interface for site hasher
     */
    class SiteHasher {
    public:
        /** Attached context */
        const PauliContext& context;

         /** Number of qubits in this particular hasher instance. */
        const size_t qubits;

        /** The size the major index (i.e. column size), in lattice mode. */
        const size_t column_height;

        /** The total number of columns (i.e. row size), in lattice mode. */
        const size_t row_width;

    protected:
        /**
         * Construct a site-hasher with information known already
         * @param qubit_count The maximum number of qubits in the hasher.
         * @param col_size The number of qubits in a column.
         */
        explicit SiteHasher(const PauliContext& context);

    public:
        /**
         * Virtual destructor for abstract base class.
         */
        virtual ~SiteHasher() noexcept = default;

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
        [[nodiscard]] static std::unique_ptr<SiteHasher> make(const PauliContext& context);
    };
}