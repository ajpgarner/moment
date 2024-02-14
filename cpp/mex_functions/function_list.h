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

    class MTKFunction;

    enum class MTKEntryPointID : int {
        Unknown = 0,
        AlgebraicMatrixSystem,
        AlphabeticName,
        ApplyMomentRules,
        CollinsGisin,
        ConvertTensor,
        Commutator,
        CommutatorMatrix,
        Complete,
        Conjugate,
        CreateMomentRules,
        EchoOperand,
        EchoMatrix,
        ExtendedMatrix,
        FlattenIndices,
        FullCorrelator,
        GenerateBasis,
        ImportMatrix,
        ImportedMatrixSystem,
        InflationMatrixSystem,
        LatticeSymmetrize,
        List,
        LocalityMatrixSystem,
        LocalizingMatrix,
        Logging,
        MakeExplicit,
        MakeRepresentation,
        MomentMatrix,
        MomentRules,
        MomentRuleSuperset,
        Multiply,
        OperatorMatrix,
        OperatorRules,
        PauliMatrixSystem,
        Plus,
        ProbabilityTable,
        Release,
        Settings,
        Simplify,
        SubstitutedMatrix,
        SuggestExtensions,
        SymbolTable,
        SymmetrizedMatrixSystem,
        TransformMatrix,
        TransformSymbols,
        ValueMatrix,
        Version,
        WordList
    };

    /**
     * Return the ID of the entry point for a given string representation.
     * @param str The string to compare
     * @return The ID of the entry point (if any match).
     */
    [[nodiscard]] MTKEntryPointID which_entrypoint(const std::string& str);

    /**
     * Return the string an the entry point for a given ID
     * @param id ID of the entry point.
     * @return The string (if any match), "unknown" otherwise.
     */
    [[nodiscard]] std::string which_function_name(MTKEntryPointID);

    /**
     *
     * @param engine The MATLAB engine object
     * @param id The MEXEntryPointID of the function to create
     * @return A unique pointer to a newly-constructed MexFunction object.
     */
    std::unique_ptr<MTKFunction> make_mtk_function(matlab::engine::MATLABEngine& engine,
                                                   MTKEntryPointID id,
                                                   StorageManager& storageManager);
}