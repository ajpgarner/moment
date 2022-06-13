/**
 * mex_entry.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Entry point for NPATK mex function suite.
 */

#include "mex.hpp"
#include "mexAdapter.hpp"

#include "mex_main.h"

class MexFunction : public matlab::mex::Function {
    public:
        void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) override {
            using namespace NPATK::mex;
            MexMain executor{getEngine()};

            executor(IOArgumentRange(outputs.begin(), outputs.begin() + outputs.size()),
                     IOArgumentRange(inputs.begin(), inputs.begin() + inputs.size()));
        }
};
