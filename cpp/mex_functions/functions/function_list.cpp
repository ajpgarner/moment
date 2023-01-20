/**
 * function_list.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_list.h"
#include "mex_function.h"

#include "functions/alphabetic_name.h"
#include "functions/apply_values.h"
#include "functions/collins_gisin.h"
#include "functions/complete.h"
#include "functions/extended_matrix.h"
#include "functions/generate_basis.h"
#include "functions/import_matrix.h"
#include "functions/list.h"
#include "functions/localizing_matrix.h"
#include "functions/moment_matrix.h"
#include "functions/new_algebraic_matrix_system.h"
#include "functions/new_imported_matrix_system.h"
#include "functions/new_inflation_matrix_system.h"
#include "functions/new_locality_matrix_system.h"
#include "functions/operator_matrix.h"
#include "functions/probability_table.h"
#include "functions/release.h"
#include "functions/rules.h"
#include "functions/symbol_table.h"
#include "functions/version.h"

#include "utilities/reporting.h"

#include <cassert>


namespace Moment::mex::functions {

    std::unique_ptr<MexFunction> make_mex_function(matlab::engine::MATLABEngine& engine,
                                                   MEXEntryPointID function_id,
                                                   StorageManager& storageManager) {
        std::unique_ptr<functions::MexFunction> the_function;

        switch(function_id) {
            case functions::MEXEntryPointID::AlphabeticName:
                the_function = std::make_unique<functions::AlphabeticName>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ApplyValues:
                the_function = std::make_unique<functions::ApplyValues>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::CollinsGisin:
                the_function = std::make_unique<functions::CollinsGisin>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Complete:
                the_function = std::make_unique<functions::Complete>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ExtendedMatrix:
                the_function = std::make_unique<functions::ExtendedMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::GenerateBasis:
                the_function = std::make_unique<functions::GenerateBasis>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ImportMatrix:
                the_function = std::make_unique<functions::ImportMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::List:
                the_function = std::make_unique<functions::List>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::LocalizingMatrix:
                the_function = std::make_unique<functions::LocalizingMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MomentMatrix:
                the_function = std::make_unique<functions::MomentMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::NewAlgebraicMatrixSystem:
                the_function = std::make_unique<functions::NewAlgebraicMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::NewImportedMatrixSystem:
                the_function = std::make_unique<functions::NewImportedMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::NewInflationMatrixSystem:
                the_function = std::make_unique<functions::NewInflationMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::NewLocalityMatrixSystem:
                the_function = std::make_unique<functions::NewLocalityMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::OperatorMatrix:
                the_function = std::make_unique<functions::OperatorMatrix>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ProbabilityTable:
                the_function = std::make_unique<functions::ProbabilityTable>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Release:
                the_function  = std::make_unique<functions::Release>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Rules:
                the_function  = std::make_unique<functions::Rules>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::SymbolTable:
                the_function = std::make_unique<functions::SymbolTable>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Unknown:
                return {};
        }

        assert(the_function->function_id == function_id);

        return the_function;
    }

    std::map<std::basic_string<char16_t>, MEXEntryPointID> make_str_to_entrypoint_map() {
        std::map<std::basic_string<char16_t>, MEXEntryPointID> output;

        output.emplace(u"alphabetic_name", MEXEntryPointID::AlphabeticName);
        output.emplace(u"apply_values", MEXEntryPointID::ApplyValues);
        output.emplace(u"collins_gisin",   MEXEntryPointID::CollinsGisin);
        output.emplace(u"complete",        MEXEntryPointID::Complete);
        output.emplace(u"extended_matrix", MEXEntryPointID::ExtendedMatrix);
        output.emplace(u"generate_basis",  MEXEntryPointID::GenerateBasis);
        output.emplace(u"list",  MEXEntryPointID::List);
        output.emplace(u"localizing_matrix",  MEXEntryPointID::LocalizingMatrix);
        output.emplace(u"import_matrix",   MEXEntryPointID::ImportMatrix);
        output.emplace(u"moment_matrix",   MEXEntryPointID::MomentMatrix);
        output.emplace(u"operator_matrix", MEXEntryPointID::OperatorMatrix);
        output.emplace(u"new_algebraic_matrix_system", MEXEntryPointID::NewAlgebraicMatrixSystem);
        output.emplace(u"new_imported_matrix_system", MEXEntryPointID::NewImportedMatrixSystem);
        output.emplace(u"new_inflation_matrix_system", MEXEntryPointID::NewInflationMatrixSystem);
        output.emplace(u"new_locality_matrix_system",  MEXEntryPointID::NewLocalityMatrixSystem);
        output.emplace(u"probability_table", MEXEntryPointID::ProbabilityTable);
        output.emplace(u"release",         MEXEntryPointID::Release);
        output.emplace(u"rules",           MEXEntryPointID::Rules);
        output.emplace(u"symbol_table",    MEXEntryPointID::SymbolTable);
        output.emplace(u"version",         MEXEntryPointID::Version);
        return output;
    }

    MEXEntryPointID which_entrypoint(const std::basic_string<char16_t> &str) {
        static const auto the_map = make_str_to_entrypoint_map();
        auto iter = the_map.find(str);
        if (iter == the_map.cend()) {
            return MEXEntryPointID::Unknown;
        }
        return iter->second;
    }
}