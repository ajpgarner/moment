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

    protected:
        const PauliContext& pauliContext;

    public:
        PauliSequenceGenerator(const PauliContext& context, size_t word_length);

    };


}