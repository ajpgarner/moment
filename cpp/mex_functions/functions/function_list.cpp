/**
 * function_list.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_list.h"
#include "mex_function.h"

#include "functions/alphabetic_name.h"
#include "functions/generate_basis.h"
#include "functions/get_symbol_table.h"
#include "functions/probability_table.h"
#include "functions/make_matrix_system.h"
#include "functions/make_moment_matrix.h"
#include "functions/make_hermitian.h"
#include "functions/make_symmetric.h"
#include "functions/release.h"
#include "functions/version.h"

#include "utilities/reporting.h"


namespace NPATK::mex::functions {

    std::unique_ptr<MexFunction> make_mex_function(matlab::engine::MATLABEngine& engine,
                                                   MEXEntryPointID function_id,
                                                   StorageManager& storageManager) {
        std::unique_ptr<functions::MexFunction> the_function;

        switch(function_id) {
            case functions::MEXEntryPointID::AlphabeticName:
                the_function = std::make_unique<functions::AlphabeticName>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::GenerateBasis:
                the_function = std::make_unique<functions::GenerateBasis>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::GetSymbolTable:
                the_function = std::make_unique<functions::GetSymbolTable>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ProbabilityTable:
                the_function = std::make_unique<functions::ProbabilityTable>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MakeHermitian:
                the_function = std::make_unique<functions::MakeHermitian>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MakeMatrixSystem:
                the_function = std::make_unique<functions::MakeMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MakeMomentMatrix:
                the_function = std::make_unique<functions::MakeMomentMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MakeSymmetric:
                the_function = std::make_unique<functions::MakeSymmetric>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Release:
                the_function  = std::make_unique<functions::Release>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Unknown:
                return {};
        }
        return the_function;
    }
}