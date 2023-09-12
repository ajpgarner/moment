/**
 * function_list.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "function_list.h"
#include "mtk_function.h"

#include "functions/conjugate.h"
#include "functions/generate_basis.h"
#include "functions/import_matrix.h"
#include "functions/make_representation.h"
#include "functions/release.h"
#include "functions/settings.h"
#include "functions/simplify.h"
#include "functions/suggest_extensions.h"
#include "functions/symbol_table.h"
#include "functions/transform_symbols.h"
#include "functions/word_list.h"
#include "functions/debug/echo.h"
#include "functions/debug/list.h"
#include "functions/debug/logging.h"
#include "functions/debug/moment_rule_superset.h"
#include "functions/debug/version.h"
#include "functions/matrix_system/algebraic_matrix_system.h"
#include "functions/matrix_system/imported_matrix_system.h"
#include "functions/matrix_system/inflation_matrix_system.h"
#include "functions/matrix_system/locality_matrix_system.h"
#include "functions/matrix_system/symmetrized_matrix_system.h"
#include "functions/moment_rules/apply_moment_rules.h"
#include "functions/moment_rules/create_moment_rules.h"
#include "functions/moment_rules/moment_rules.h"
#include "functions/operator_matrix/substituted_matrix.h"
#include "functions/operator_matrix/extended_matrix.h"
#include "functions/operator_matrix/localizing_matrix.h"
#include "functions/operator_matrix/moment_matrix.h"
#include "functions/operator_matrix/operator_matrix.h"
#include "functions/operator_rules/complete.h"
#include "functions/operator_rules/operator_rules.h"
#include "functions/probability/collins_gisin.h"
#include "functions/probability/full_correlator.h"
#include "functions/probability/make_explicit.h"
#include "functions/probability/probability_table.h"
#include "functions/utility/alphabetic_name.h"
#include "functions/utility/flatten_indices.h"
#include "utilities/reporting.h"

#include <cassert>


namespace Moment::mex::functions {
    namespace {

        std::map<std::string, MTKEntryPointID> make_str_to_entrypoint_map() {
            std::map<std::string, MTKEntryPointID> output;
            output.emplace("algebraic_matrix_system",   MTKEntryPointID::AlgebraicMatrixSystem);
            output.emplace("alphabetic_name", MTKEntryPointID::AlphabeticName);
            output.emplace("apply_moment_rules", MTKEntryPointID::ApplyMomentRules);
            output.emplace("collins_gisin",   MTKEntryPointID::CollinsGisin);
            output.emplace("complete",        MTKEntryPointID::Complete);
            output.emplace("conjugate",       MTKEntryPointID::Conjugate);
            output.emplace("create_moment_rules",   MTKEntryPointID::CreateMomentRules);
            output.emplace("echo",            MTKEntryPointID::Echo);
            output.emplace("extended_matrix", MTKEntryPointID::ExtendedMatrix);
            output.emplace("flatten_indices", MTKEntryPointID::FlattenIndices);
            output.emplace("full_correlator", MTKEntryPointID::FullCorrelator);
            output.emplace("generate_basis",  MTKEntryPointID::GenerateBasis);
            output.emplace("list",            MTKEntryPointID::List);
            output.emplace("import_matrix",   MTKEntryPointID::ImportMatrix);
            output.emplace("imported_matrix_system",    MTKEntryPointID::ImportedMatrixSystem);
            output.emplace("inflation_matrix_system",   MTKEntryPointID::InflationMatrixSystem);
            output.emplace("localizing_matrix",  MTKEntryPointID::LocalizingMatrix);
            output.emplace("locality_matrix_system",    MTKEntryPointID::LocalityMatrixSystem);
            output.emplace("logging",  MTKEntryPointID::Logging);
            output.emplace("make_explicit",      MTKEntryPointID::MakeExplicit);
            output.emplace("make_representation",MTKEntryPointID::MakeRepresentation);
            output.emplace("moment_rules",       MTKEntryPointID::MomentRules);
            output.emplace("moment_matrix",      MTKEntryPointID::MomentMatrix);
            output.emplace("moment_rule_superset",      MTKEntryPointID::MomentRuleSuperset);
            output.emplace("operator_matrix",    MTKEntryPointID::OperatorMatrix);
            output.emplace("operator_rules",     MTKEntryPointID::OperatorRules);
            output.emplace("probability_table",  MTKEntryPointID::ProbabilityTable);
            output.emplace("release",            MTKEntryPointID::Release);
            output.emplace("settings",           MTKEntryPointID::Settings);
            output.emplace("simplify",           MTKEntryPointID::Simplify);
            output.emplace("substituted_matrix", MTKEntryPointID::SubstitutedMatrix);
            output.emplace("suggest_extensions", MTKEntryPointID::SuggestExtensions);
            output.emplace("symmetrized_matrix_system", MTKEntryPointID::SymmetrizedMatrixSystem);
            output.emplace("symbol_table",       MTKEntryPointID::SymbolTable);
            output.emplace("transform_symbols",  MTKEntryPointID::TransformSymbols);
            output.emplace("version",            MTKEntryPointID::Version);
            output.emplace("word_list",          MTKEntryPointID::WordList);
            return output;
        }

        std::map<MTKEntryPointID, std::string> make_entrypoint_to_str_map() {
            std::map<MTKEntryPointID, std::string> output;
            auto fwd_map = make_str_to_entrypoint_map();
            for (auto [str, entry] : fwd_map) {
                auto [key, did_insert] = output.insert(std::make_pair(entry, str));
                assert(did_insert);
            }
            return output;
        }
    }


    MTKEntryPointID which_entrypoint(const std::string &str) {
        static const auto the_map = make_str_to_entrypoint_map();
        auto iter = the_map.find(str);
        if (iter == the_map.cend()) {
            return MTKEntryPointID::Unknown;
        }
        return iter->second;
    }

    std::string which_function_name(MTKEntryPointID id) {
        static const auto the_map = make_entrypoint_to_str_map();
        auto iter = the_map.find(id);
        if (iter == the_map.cend()) {
            return "unknown";
        }
        return iter->second;
    }

    std::unique_ptr<MTKFunction> make_mtk_function(matlab::engine::MATLABEngine& engine,
                                                   MTKEntryPointID function_id,
                                                   StorageManager& storageManager) {
        std::unique_ptr<functions::MTKFunction> the_function;

        switch(function_id) {
            case functions::MTKEntryPointID::AlgebraicMatrixSystem:
                the_function = std::make_unique<functions::AlgebraicMatrixSystem>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::AlphabeticName:
                the_function = std::make_unique<functions::AlphabeticName>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::ApplyMomentRules:
                the_function = std::make_unique<functions::ApplyMomentRules>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::CollinsGisin:
                the_function = std::make_unique<functions::CollinsGisin>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Complete:
                the_function = std::make_unique<functions::Complete>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Conjugate:
                the_function = std::make_unique<functions::Conjugate>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::CreateMomentRules:
                the_function = std::make_unique<functions::CreateMomentRules>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Echo:
                the_function = std::make_unique<functions::Echo>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::ExtendedMatrix:
                the_function = std::make_unique<functions::ExtendedMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::FlattenIndices:
                the_function = std::make_unique<functions::FlattenIndices>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::FullCorrelator:
                the_function = std::make_unique<functions::FullCorrelator>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::GenerateBasis:
                the_function = std::make_unique<functions::GenerateBasis>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::ImportMatrix:
                the_function = std::make_unique<functions::ImportMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::ImportedMatrixSystem:
                the_function = std::make_unique<functions::ImportedMatrixSystem>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::InflationMatrixSystem:
                the_function = std::make_unique<functions::InflationMatrixSystem>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::List:
                the_function = std::make_unique<functions::List>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::LocalityMatrixSystem:
                the_function = std::make_unique<functions::LocalityMatrixSystem>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::LocalizingMatrix:
                the_function = std::make_unique<functions::LocalizingMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Logging:
                the_function = std::make_unique<functions::Logging>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::MakeExplicit:
                the_function = std::make_unique<functions::MakeExplicit>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::MakeRepresentation:
                the_function = std::make_unique<functions::MakeRepresentation>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::MomentMatrix:
                the_function = std::make_unique<functions::MomentMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::MomentRules:
                the_function = std::make_unique<functions::MomentRules>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::MomentRuleSuperset:
                the_function = std::make_unique<functions::MomentRuleSuperset>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::OperatorMatrix:
                the_function = std::make_unique<functions::RawOperatorMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::OperatorRules:
                the_function = std::make_unique<functions::OperatorRules>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::ProbabilityTable:
                the_function = std::make_unique<functions::ProbabilityTable>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Release:
                the_function  = std::make_unique<functions::Release>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Settings:
                the_function = std::make_unique<functions::Settings>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Simplify:
                the_function = std::make_unique<functions::Simplify>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::SubstitutedMatrix:
                the_function = std::make_unique<functions::SubstitutedMatrix>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::SuggestExtensions:
                the_function = std::make_unique<functions::SuggestExtensions>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::SymbolTable:
                the_function = std::make_unique<functions::SymbolTable>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::SymmetrizedMatrixSystem:
                the_function = std::make_unique<functions::SymmetrizedMatrixSystem>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::TransformSymbols:
                the_function = std::make_unique<functions::TransformSymbols>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::WordList:
                the_function = std::make_unique<functions::WordList>(engine, storageManager);
                break;
            case functions::MTKEntryPointID::Unknown:
                return {};
        }

        assert(the_function->function_id == function_id);

        return the_function;
    }


}