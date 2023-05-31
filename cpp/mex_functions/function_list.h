/**
 * function_list.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <string>
#include <map>
#include <memory>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {
    class StorageManager;
}

namespace Moment::mex::functions {

    class MexFunction;

    enum class MEXEntryPointID : int {
        Unknown = 0,
        AlgebraicMatrixSystem,
        AlphabeticName,
        ApplyMomentRules,
        CollinsGisin,
        Complete,
        Conjugate,
        CreateMomentRules,
        Echo,
        ExtendedMatrix,
        GenerateBasis,
        ImportMatrix,
        ImportedMatrixSystem,
        InflationMatrixSystem,
        List,
        LocalityMatrixSystem,
        LocalizingMatrix,
        MakeExplicit,
        MakeRepresentation,
        MomentMatrix,
        MonomialRules,
        OperatorMatrix,
        ProbabilityTable,
        Release,
        Settings,
        Simplify,
        SubstitutedMatrix,
        SuggestExtensions,
        SymbolTable,
        SymmetrizedMatrixSystem,
        TransformSymbols,
        Version,
        WordList
    };

    /**
     * Return the ID of the entry point for a given string representation.
     * @param str The string to compare
     * @return The ID of the entry point (if any match).
     */
    [[nodiscard]] MEXEntryPointID which_entrypoint(const std::string& str);

    /**
     * Return the string an the entry point for a given ID
     * @param id ID of the entry point.
     * @return The string (if any match), "unknown" otherwise.
     */
    [[nodiscard]] std::string which_function_name(MEXEntryPointID);

    /**
     *
     * @param engine The MATLAB engine object
     * @param id The MEXEntryPointID of the function to create
     * @return A unique pointer to a newly-constructed MexFunction object.
     */
    std::unique_ptr<MexFunction> make_mex_function(matlab::engine::MATLABEngine& engine,
                                                   MEXEntryPointID id,
                                                   StorageManager& storageManager);
}