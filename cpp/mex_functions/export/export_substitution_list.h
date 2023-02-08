/**
 * export_substitution_list.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "integer_types.h"

#include <map>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {
    matlab::data::Array export_substitution_list(matlab::engine::MATLABEngine &engine,
                                                 const std::map<symbol_name_t, double>& substitutions);
}
