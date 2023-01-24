/**
 * environmental_variables.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "environmental_variables.h"

#include "scenarios/locality/locality_operator_formatter.h"

#include <cassert>

namespace Moment::mex {
    EnvironmentalVariables::EnvironmentalVariables() {
        // Default to natural format of locality operators
        this->the_l_op_formatter = std::make_shared<Locality::NaturalLOFormatter>();
    }

    EnvironmentalVariables::~EnvironmentalVariables() = default;

    std::shared_ptr<Locality::LocalityOperatorFormatter> EnvironmentalVariables::get_locality_formatter() const {
        assert(this->the_l_op_formatter);
        return this->the_l_op_formatter;
    }

    void EnvironmentalVariables::set_locality_formatter(std::shared_ptr<Locality::LocalityOperatorFormatter> lof) {
        assert(lof);
        this->the_l_op_formatter = std::move(lof);
    }


}