/**
 * inflation_osg.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence_generator.h"

namespace Moment::Inflation {

    class InflationContext;

    class InflationOperatorSequenceGenerator : public OperatorSequenceGenerator {

    protected:
        const InflationContext& inflationContext;

    public:
        InflationOperatorSequenceGenerator(const InflationContext& context, size_t word_length);

    private:
        void generate_completely_projective();

        void generate_not_completely_projective();
    };


}