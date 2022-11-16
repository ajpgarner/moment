/**
 * inflation_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"

#include "causal_network.h"

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace NPATK {


    class InflationContext : public Context {
    private:
        CausalNetwork base_network;
        size_t inflation;

    public:
        /**
         * Create a causal network context, for inflating.
         * @param network Causal network
         */
        InflationContext(CausalNetwork network, size_t inflation_level);

        /**
         * Vector of observables associated with context.
         */
        [[nodiscard]] const auto& Observables() const noexcept { return this->base_network.Observables(); }

        /**
         * Vector of sources associated with context.
         */
        [[nodiscard]] const auto& Sources() const noexcept { return this->base_network.Sources(); }

        /**
         * Level of inflation
         */
         size_t Inflation() const noexcept { return this->inflation; }

         /**
          * Output inflation about context
          */
        std::string to_string() const override;

    };
}