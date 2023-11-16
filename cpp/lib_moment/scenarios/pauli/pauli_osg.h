/**
 * pauli_osg.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "dictionary/operator_sequence_generator.h"

namespace Moment::Pauli {

    class PauliContext;

    class PauliSequenceGenerator : public OperatorSequenceGenerator {
    public:
        const PauliContext& pauliContext;

        /**
         * The maximum (inclusive) distance between qubits to include in a sequence.
         */
         const size_t nearest_neighbours = 0;

        /**
         * Is qubit (N-1) considered to be adjacent to qubit 0 ?
         */
        const bool wrap = false;

    public:
        /**
         * Generate all operators up to word length.
         */
        PauliSequenceGenerator(const PauliContext& context, size_t word_length);

        /**
         * Generate only nearest neighbour operators up to word length.
         */
        PauliSequenceGenerator(const PauliContext& context, size_t word_length, size_t neighbours, bool wrap = false);

        /** True, if only a limited subset of sequences are considered (i.e. nearest neighbour mode). */
        [[nodiscard]] constexpr bool limited() const noexcept { return this->nearest_neighbours != 0; }

    };


}