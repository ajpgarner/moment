/**
 * export_substitution_list.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_substitution_list.h"
#include "reporting.h"

namespace NPATK::mex {
    matlab::data::Array export_substitution_list(matlab::engine::MATLABEngine& engine,
                                                       const SymbolTree& tree) {

        NPATK::mex::throw_error(engine, "Not implemented.");
        throw;
    }
}