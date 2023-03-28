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
        AlphabeticName,
        ApplyValues,
        CollinsGisin,
        Complete,
        Conjugate,
        Echo,
        ExtendedMatrix,
        GenerateBasis,
        ImportMatrix,
        List,
        LocalizingMatrix,
        MakeExplicit,
        MakeRepresentation,
        MomentMatrix,
        NewAlgebraicMatrixSystem,
        NewImportedMatrixSystem,
        NewInflationMatrixSystem,
        NewLocalityMatrixSystem,
        NewSymmetrizedMatrixSystem,
        OperatorMatrix,
        ProbabilityTable,
        Release,
        Rules,
        Settings,
        SuggestExtensions,
        SymbolTable,
        Version
    };

    /**
     * Register association between strings and entry point IDs.
     * @return
     */
    std::map<std::basic_string<char16_t>, MEXEntryPointID> make_str_to_entrypoint_map();

    /**
     * Return the ID of the entry point for a given string representation.
     * @param str The string to compare
     * @return The ID of the entry point (if any match).
     */
    MEXEntryPointID which_entrypoint(const std::basic_string<char16_t>& str);

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