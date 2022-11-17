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
        struct ICOperatorInfo {
            oper_name_t global_id;
            oper_name_t observable;
            oper_name_t flattenedSourceIndex;
            oper_name_t outcome;

        public:
            ICOperatorInfo(oper_name_t id, oper_name_t observable, oper_name_t flattenedIndex, oper_name_t outcome)
                : global_id{id}, observable{observable}, flattenedSourceIndex{flattenedIndex}, outcome{outcome} { }
        };

    private:
        CausalNetwork base_network;
        size_t inflation;
        std::vector<ICOperatorInfo> operator_info;

    public:
        bool additional_simplification(std::vector<oper_name_t> &op_sequence, bool &negate) const override;

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
         [[nodiscard]] size_t Inflation() const noexcept { return this->inflation; }

         /**
          * Output inflation about context
          */
         [[nodiscard]] std::string to_string() const override;

    };
}