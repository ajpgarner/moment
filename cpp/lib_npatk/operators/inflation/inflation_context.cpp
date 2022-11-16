/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

#include <sstream>

namespace NPATK {

    InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

    }

    std::string InflationContext::to_string() const {
        std::stringstream ss;
        ss << "Inflation setting with "
           << this->operators.size() << ((1 !=  this->operators.size()) ? " operators" : " operator")
           << " in total.\n\n";
        ss << this->base_network << "\n";
        ss << "Inflation level: " << this->inflation;

        return ss.str();
    }
}
