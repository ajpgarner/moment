/**
 * function_list.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_list.h"
#include "mex_function.h"

#include "functions/version.h"
#include "functions/make_symmetric.h"
#include "functions/make_hermitian.h"
#include "functions/generate_basis.h"
#include "functions/make_moment_matrix.h"
#include "functions/alphabetic_name.h"

#include "utilities/reporting.h"


namespace NPATK::mex::functions {

    std::unique_ptr<MexFunction> make_mex_function(matlab::engine::MATLABEngine& engine, MEXEntryPointID function_id) {
        std::unique_ptr<functions::MexFunction> the_function;

        switch(function_id) {
            case functions::MEXEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(engine);
                break;
            case functions::MEXEntryPointID::MakeSymmetric:
                the_function = std::make_unique<functions::MakeSymmetric>(engine);
                break;
            case functions::MEXEntryPointID::MakeHermitian:
                the_function = std::make_unique<functions::MakeHermitian>(engine);
                break;
            case functions::MEXEntryPointID::GenerateBasis:
                the_function = std::make_unique<functions::GenerateBasis>(engine);
                break;
            case functions::MEXEntryPointID::MakeMomentMatrix:
                the_function = std::make_unique<functions::MakeMomentMatrix>(engine);
                break;
            case functions::MEXEntryPointID::AlphabeticName:
                the_function = std::make_unique<functions::AlphabeticName>(engine);
                break;
            default:
            case functions::MEXEntryPointID::Unknown:
                return {};
        }
        return the_function;
    }
}