/**
 * export_monomial_rules.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

namespace NPATK {
    class RuleBook;
}

namespace NPATK::mex {
    matlab::data::CellArray export_monomial_rules(const RuleBook& rules);
}