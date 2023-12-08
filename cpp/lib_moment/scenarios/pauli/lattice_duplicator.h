/**
 * lattice_duplicator.h
 *
 * Part of the OSG process
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence.h"
#include <vector>

namespace Moment::Pauli {
    class PauliContext;

    class LatticeDuplicator {
    public:
        const PauliContext& context;

    private:
        std::vector<OperatorSequence>& output;

    public:
        LatticeDuplicator(const PauliContext& context, std::vector<OperatorSequence>& output)
            : context{context}, output{output} { }

        /**
         * Adds all 3 variants of supplied qubit to the output.
         * Undefined behaviour if index is out of bounds.
         */
        void one_qubit_fill(size_t qubit_index);

        /**
         * Adds all 9 variants of supplied pair of qubits to the output.
         * Undefined behaviour if indices are the same, or are out of bounds.
         */
        void two_qubit_fill(size_t first_site_index, size_t second_site_index);

        /**
         * Adds all permutations of Pauli operators on supplied list of lattice sites to the output.
         * Undefined behaviour if any site indices overlap, or are out of bounds.
         * @return Pair with index to first and one-past last element added to output list.
         */
        std::pair<size_t, size_t> permutation_fill(std::span<const size_t> lattice_sites);

        /**
         * Adds all unique offsets of supplied input sequence to the output.
         * Undefined behaviour if any site indices overlap, or are out of bounds.
         * @return Pair with index to first and one-past last element added to output list.
         */
        std::pair<size_t, size_t> symmetrical_fill(std::span<const size_t> lattice_sites,
                                                   bool check_for_aliases = false);

        /**
         * Adds all unique offsets of supplied input sequence to the output, but do not wrap around.
         * Undefined behaviour if any site indices overlap, or are out of bounds.
         * @return Pair with index to first and one-past last element added to output list.
         */
        std::pair<size_t, size_t> wrapless_symmetrical_fill(std::span<const size_t> lattice_sites);


    };



}