/**
 * pauli_osg.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "dictionary/operator_sequence_generator.h"
#include "nearest_neighbour_index.h"

namespace Moment::Pauli {

    class PauliContext;

    /**
     * Generates operator sequences of up to a desired length for the Pauli scenario.
     * Can be constrained to only generate series whose operators are nearest neighbours, or N-nearest neighbours.
     */
    class PauliSequenceGenerator : public OperatorSequenceGenerator {
    public:
        const PauliContext& pauliContext;

        /**
         * The OSG index
         */
        const NearestNeighbourIndex nearest_neighbour_index;


    public:
        /**
         * Generate all operators up to word length.
         */
        PauliSequenceGenerator(const PauliContext& context, size_t word_length);

        /**
         * Generate only nearest neighbour operators up to word length.
         */
        PauliSequenceGenerator(const PauliContext& context, const NearestNeighbourIndex& index);

        /**
         * Generate only nearest neighbour operators up to word length.
         */
        PauliSequenceGenerator(const PauliContext& context, size_t word_length, size_t neighbours, bool wrap = false)
            : PauliSequenceGenerator{context,
                                     NearestNeighbourIndex{word_length, neighbours, wrap ? (neighbours > 0) : false}} {
        }


        /** True, if only a limited subset of sequences are considered (i.e. nearest neighbour mode). */
        [[nodiscard]] constexpr bool limited() const noexcept { return this->nearest_neighbour_index.neighbours != 0; }

    };


}