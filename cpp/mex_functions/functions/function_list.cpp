/**
 * function_list.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "function_list.h"
#include "mex_function.h"

#include "functions/alphabetic_name.h"
#include "functions/apply_moment_rules.h"
#include "functions/collins_gisin.h"
#include "functions/complete.h"
#include "functions/conjugate.h"
#include "functions/create_moment_rules.h"
#include "functions/echo.h"
#include "functions/extended_matrix.h"
#include "functions/generate_basis.h"
#include "functions/import_matrix.h"
#include "functions/list.h"
#include "functions/localizing_matrix.h"
#include "functions/make_explicit.h"
#include "functions/make_representation.h"
#include "functions/moment_matrix.h"
#include "functions/new_algebraic_matrix_system.h"
#include "functions/new_imported_matrix_system.h"
#include "functions/new_inflation_matrix_system.h"
#include "functions/new_locality_matrix_system.h"
#include "functions/new_symmetrized_matrix_system.h"
#include "functions/operator_matrix.h"
#include "functions/probability_table.h"
#include "functions/release.h"
#include "functions/rules.h"
#include "functions/settings.h"
#include "functions/simplify.h"
#include "functions/suggest_extensions.h"
#include "functions/symbol_table.h"
#include "functions/transform_symbols.h"
#include "functions/version.h"
#include "functions/word_list.h"

#include "utilities/reporting.h"

#include <cassert>


namespace Moment::mex::functions {
    namespace {

        std::map<std::string, MEXEntryPointID> make_str_to_entrypoint_map() {
            std::map<std::string, MEXEntryPointID> output;
            output.emplace("alphabetic_name", MEXEntryPointID::AlphabeticName);
            output.emplace("apply_moment_rules",    MEXEntryPointID::ApplyMomentRules);
            output.emplace("collins_gisin",   MEXEntryPointID::CollinsGisin);
            output.emplace("complete",        MEXEntryPointID::Complete);
            output.emplace("conjugate",       MEXEntryPointID::Conjugate);
            output.emplace("create_moment_rules",   MEXEntryPointID::CreateMomentRules);
            output.emplace("echo",            MEXEntryPointID::Echo);
            output.emplace("extended_matrix", MEXEntryPointID::ExtendedMatrix);
            output.emplace("generate_basis",  MEXEntryPointID::GenerateBasis);
            output.emplace("list",            MEXEntryPointID::List);
            output.emplace("localizing_matrix",  MEXEntryPointID::LocalizingMatrix);
            output.emplace("import_matrix",   MEXEntryPointID::ImportMatrix);
            output.emplace("make_explicit",   MEXEntryPointID::MakeExplicit);
            output.emplace("make_representation",   MEXEntryPointID::MakeRepresentation);
            output.emplace("moment_matrix",   MEXEntryPointID::MomentMatrix);
            output.emplace("operator_matrix", MEXEntryPointID::OperatorMatrix);
            output.emplace("new_algebraic_matrix_system",   MEXEntryPointID::NewAlgebraicMatrixSystem);
            output.emplace("new_imported_matrix_system",    MEXEntryPointID::NewImportedMatrixSystem);
            output.emplace("new_inflation_matrix_system",   MEXEntryPointID::NewInflationMatrixSystem);
            output.emplace("new_locality_matrix_system",    MEXEntryPointID::NewLocalityMatrixSystem);
            output.emplace("new_symmetrized_matrix_system", MEXEntryPointID::NewSymmetrizedMatrixSystem);
            output.emplace("probability_table",  MEXEntryPointID::ProbabilityTable);
            output.emplace("release",            MEXEntryPointID::Release);
            output.emplace("rules",              MEXEntryPointID::Rules);
            output.emplace("settings",           MEXEntryPointID::Settings);
            output.emplace("simplify",           MEXEntryPointID::Simplify);
            output.emplace("suggest_extensions", MEXEntryPointID::SuggestExtensions);
            output.emplace("symbol_table",       MEXEntryPointID::SymbolTable);
            output.emplace("transform_symbols",  MEXEntryPointID::TransformSymbols);
            output.emplace("version",            MEXEntryPointID::Version);
            output.emplace("word_list",          MEXEntryPointID::WordList);
            return output;
        }

        std::map<MEXEntryPointID, std::string> make_entrypoint_to_str_map() {
            std::map<MEXEntryPointID, std::string> output;
            auto fwd_map = make_str_to_entrypoint_map();
            for (auto [str, entry] : fwd_map) {
                output.insert(std::make_pair(entry, str));
            }
            return output;
        }
    }


    MEXEntryPointID which_entrypoint(const std::string &str) {
        static const auto the_map = make_str_to_entrypoint_map();
        auto iter = the_map.find(str);
        if (iter == the_map.cend()) {
            return MEXEntryPointID::Unknown;
        }
        return iter->second;
    }

    std::string which_function_name(MEXEntryPointID id) {
        static const auto the_map = make_entrypoint_to_str_map();
        auto iter = the_map.find(id);
        if (iter == the_map.cend()) {
            return "unknown";
        }
        return iter->second;
    }

    std::unique_ptr<MexFunction> make_mex_function(matlab::engine::MATLABEngine& engine,
                                                   MEXEntryPointID function_id,
                                                   StorageManager& storageManager) {
        std::unique_ptr<functions::MexFunction> the_function;

        switch(function_id) {
            case functions::MEXEntryPointID::AlphabeticName:
                the_function = std::make_unique<functions::AlphabeticName>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::ApplyMomentRules:
                the_function = std::make_unique<functions::ApplyMomentRules>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::CollinsGisin:
                the_function = std::make_unique<functions::CollinsGisin>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Complete:
                the_function = std::make_unique<functions::Complete>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Conjugate:
                the_function = std::make_unique<functions::Conjugate>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::CreateMomentRules:
                the_function = std::make_unique<functions::CreateMomentRules>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Echo:
                the_function = std::make_unique<functions::Echo>(engine, storageManager);
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
            case functions::MEXEntryPointID::MakeExplicit:
                the_function = std::make_unique<functions::MakeExplicit>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::MakeRepresentation:
                the_function = std::make_unique<functions::MakeRepresentation>(engine, storageManager);
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
            case functions::MEXEntryPointID::NewSymmetrizedMatrixSystem:
                the_function = std::make_unique<functions::NewSymmetrizedMatrixSystem>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::OperatorMatrix:
                the_function = std::make_unique<functions::RawOperatorMatrix>(engine, storageManager);
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
            case functions::MEXEntryPointID::Settings:
                the_function = std::make_unique<functions::Settings>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Simplify:
                the_function = std::make_unique<functions::Simplify>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::SuggestExtensions:
                the_function = std::make_unique<functions::SuggestExtensions>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::SymbolTable:
                the_function = std::make_unique<functions::SymbolTable>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::TransformSymbols:
                the_function = std::make_unique<functions::TransformSymbols>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::WordList:
                the_function = std::make_unique<functions::WordList>(engine, storageManager);
                break;
            case functions::MEXEntryPointID::Unknown:
                return {};
        }

        assert(the_function->function_id == function_id);

        return the_function;
    }


}