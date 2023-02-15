/**
 * export_monomial_rules.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

namespace Moment::Algebraic {
    class RuleBook;
}

namespace Moment::mex {
    matlab::data::CellArray export_monomial_rules(const Algebraic::RuleBook& rules, bool matlab_indices);
}