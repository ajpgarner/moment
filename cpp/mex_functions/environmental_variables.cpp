/**
 * environmental_variables.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "environmental_variables.h"

#include "scenarios/locality/locality_operator_formatter.h"

#include <cassert>

#include <iostream>

namespace Moment::mex {
    EnvironmentalVariables::EnvironmentalVariables() {
        // Default to natural format of locality operators
        this->the_l_op_formatter = std::make_shared<Locality::NaturalLOFormatter>();
        this->mt_policy = Multithreading::MultiThreadPolicy::Optional;
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

    std::ostream& operator<<(std::ostream& os, const EnvironmentalVariables& ev) {
        os << "Locality scenario operator format: " << ev.the_l_op_formatter->name() << "\n";
        os << "Multithreading policy: " << ev.mt_policy << "\n";
        return os;
    }

    EnvironmentalVariables::EnvironmentalVariables(const EnvironmentalVariables &reference) {
        // shallow copy is okay, as formatter is const shared object
        this->the_l_op_formatter = reference.the_l_op_formatter;
        this->mt_policy = reference.mt_policy;
    }

}