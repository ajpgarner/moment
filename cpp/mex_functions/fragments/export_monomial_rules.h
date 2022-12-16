/**
 * export_monomial_rules.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

namespace Moment {
    class RuleBook;
}

namespace Moment::mex {
    matlab::data::CellArray export_monomial_rules(const RuleBook& rules, bool matlab_indices);
}