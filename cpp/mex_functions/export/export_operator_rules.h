/**
 * export_operator_rules.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "exporter.h"

namespace Moment::Algebraic {
    class OperatorRulebook;
}

namespace Moment::mex {

    class OperatorRuleExporter : public Exporter {
    public:
        const bool matlab_indices = false;

        OperatorRuleExporter(matlab::engine::MATLABEngine& engine,
                             matlab::data::ArrayFactory& factory,
                             const bool matlab_indices = true)
            : Exporter{engine, factory}, matlab_indices{matlab_indices} { };

        matlab::data::CellArray operator()(const Algebraic::OperatorRulebook& rules);
    };


}