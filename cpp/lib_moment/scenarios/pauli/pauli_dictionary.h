/**
 * pauli_dictionary.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "dictionary/dictionary.h"

#include "nearest_neighbour_index.h"
#include <map>

namespace Moment::Pauli {
    class PauliContext;

    class PauliDictionary : public Dictionary {
    public:
        const PauliContext& pauliContext;

    private:
        mutable std::map<NearestNeighbourIndex, size_t> nn_indices{};

    public:

        explicit PauliDictionary(const PauliContext& context);

        /**
         * Gets a nearest neighbour partial-NPA hierarchy level generator.
         * @param npa_level The maximum word length.
         */
        [[nodiscard]] const OSGPair& NearestNeighbour(const NearestNeighbourIndex& index) const;


    };
}