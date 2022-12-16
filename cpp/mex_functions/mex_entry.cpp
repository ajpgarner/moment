/**
 * mex_entry.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Entry point for Moment mex function suite.
 */

#include "mex.hpp"
#include "mexAdapter.hpp"

#include "mex_main.h"

class MexFunction : public matlab::mex::Function {
    public:
        MexFunction() {

        }

        void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) override {
            Moment::mex::MexMain executor{getEngine()};

            executor(Moment::mex::IOArgumentRange(outputs.begin(), outputs.begin() + outputs.size()),
                     Moment::mex::IOArgumentRange(inputs.begin(), inputs.begin() + inputs.size()));
        }
};
