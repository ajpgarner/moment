/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

namespace NPATK {

    InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

    }
}
