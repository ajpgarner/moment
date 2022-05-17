/**
 * version.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_base.h"
#include "version.h"

#include "../helpers/reporting.h"

namespace NPATK::mex::functions {



    void Version::operator()(WrappedArgRange output, WrappedArgRange input) {
        debug_message(this->matlabEngine, "NPA toolkit, v0.01\nCopyright (c) 2022 Austrian Academy of Sciences\n\n");
    }
}
