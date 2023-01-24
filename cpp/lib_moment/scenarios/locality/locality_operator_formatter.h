/**
 * locality_operator_formatter.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include <iostream>
#include <string>

namespace Moment::Locality {

    class Party;
    class Measurement;


    class LocalityOperatorFormatter {
    public:
        LocalityOperatorFormatter() = default;

        virtual ~LocalityOperatorFormatter();

        /** Format operator, with party information */
        [[nodiscard]] std::string format(const Party& party, const Measurement& measurement, oper_name_t outcome) const;

        /** Format operator, without party information */
        [[nodiscard]] std::string format(const Measurement& measurement, oper_name_t outcome) const;

        /** Format operator, with party information */
        virtual std::ostream&
        format(std::ostream& os, const Party& party, const Measurement& measurement, oper_name_t outcome) const = 0;

        /** Format operator, without party information */
        virtual std::ostream&
        format(std::ostream& os, const Measurement& measurement, oper_name_t outcome) const = 0;

        /** Name of formatter */
        virtual std::string name() const = 0;
    };

    class NaturalLOFormatter : public LocalityOperatorFormatter {
    public:
        NaturalLOFormatter() = default;

        std::ostream& format(std::ostream &os,
                             const Party &party, const Measurement &measurement, oper_name_t outcome) const final;

        std::ostream& format(std::ostream &os, const Measurement &measurement, oper_name_t outcome) const final;

        std::string name() const final { return "Natural"; }
    };

    class TraditionalLOFormatter : public LocalityOperatorFormatter {
    public:
        TraditionalLOFormatter() = default;

        std::ostream& format(std::ostream &os,
                             const Party &party, const Measurement &measurement, oper_name_t outcome) const final;

        std::ostream& format(std::ostream &os, const Measurement &measurement, oper_name_t outcome) const final;

        std::string name() const final { return "Traditional"; }

    };
}