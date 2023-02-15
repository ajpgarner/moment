/**
 * locality_operator_formatter.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_operator_formatter.h"

#include "party.h"
#include "measurement.h"

#include <iostream>
#include <sstream>

namespace Moment::Locality {

    LocalityOperatorFormatter::~LocalityOperatorFormatter() = default;

    std::string
    LocalityOperatorFormatter::format(const Party &party, const Measurement &measurement, oper_name_t outcome) const {
        std::stringstream ss;
        this->format(ss, party, measurement, outcome);
        return ss.str();
    }

    std::string
    LocalityOperatorFormatter::format(const Measurement &measurement, oper_name_t outcome) const {
        std::stringstream ss;
        this->format(ss, measurement, outcome);
        return ss.str();
    }

    std::ostream& NaturalLOFormatter::format(std::ostream &os, const Party &party, const Measurement &mmt,
                                             oper_name_t outcome_num) const {
        os << party.name << "." << mmt.name << outcome_num;
        return os;
    }

    std::ostream&
    NaturalLOFormatter::format(std::ostream &os, const Measurement &mmt, oper_name_t outcome_num) const {
        os << mmt.name << outcome_num;
        return os;
    }

    std::ostream& TraditionalLOFormatter::format(std::ostream &os, const Party &party, const Measurement &mmt,
                                             oper_name_t outcome_num) const {
        os << party.name << outcome_num << "|" << mmt.Index().mmt;
        return os;
    }

    std::ostream&
    TraditionalLOFormatter::format(std::ostream &os, const Measurement &mmt, oper_name_t outcome_num) const {
        os << mmt.name << outcome_num;
        os << outcome_num << "|" << mmt.Index().mmt;
        return os;
    }
}