/**
 * export_operator_rules.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

namespace Moment::Algebraic {
    class OperatorRulebook;
}

namespace Moment::mex {
    matlab::data::CellArray export_operator_rules(const Algebraic::OperatorRulebook& rules, bool matlab_indices);
}