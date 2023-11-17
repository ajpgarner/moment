/**
 * pauli_dictionary.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_dictionary.h"
#include "pauli_context.h"

namespace Moment::Pauli {

    PauliDictionary::PauliDictionary(const Moment::Pauli::PauliContext& context_in)
        : Dictionary{context_in}, pauliContext{context_in} { }
}