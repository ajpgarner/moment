/**
 * canonical_observables.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "observable.h"

namespace NPATK {

    class InflationContext;

    class CanonicalObservables {

    private:
        const InflationContext& context;
        size_t max_level = 0;

    public:
        explicit CanonicalObservables(const InflationContext& context);

        void generate_up_to_level(size_t new_level);


    };

}