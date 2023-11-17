/**
 * pauli_dictionary.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "dictionary/dictionary.h"

namespace Moment::Pauli {
    class PauliContext;

    class PauliDictionary : public Dictionary {
    public:
        const PauliContext& pauliContext;

        explicit PauliDictionary(const PauliContext& context);

    };
}