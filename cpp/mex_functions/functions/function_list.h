/**
 * function_list.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include <map>
#include <memory>

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {
    class StorageManager;
}

namespace NPATK::mex::functions {

    enum class MEXEntryPointID : int {
        Unknown = 0,
        AlphabeticName,
        CollinsGisin,
        GenerateBasis,
        ProbabilityTable,
        MakeHermitian,
        MakeSymmetric,
        MomentMatrix,
        NewMatrixSystem,
        Release,
        SymbolTable,
        Version
    };

    /**
     * Register association between strings and entry point IDs.
     * @return
     */
    inline std::map<std::basic_string<char16_t>, MEXEntryPointID> make_str_to_entrypoint_map() {
        std::map<std::basic_string<char16_t>, MEXEntryPointID> output;

        output.emplace(u"alphabetic_name", MEXEntryPointID::AlphabeticName);
        output.emplace(u"collins_gisin", MEXEntryPointID::CollinsGisin);
        output.emplace(u"generate_basis", MEXEntryPointID::GenerateBasis);
        output.emplace(u"probability_table", MEXEntryPointID::ProbabilityTable);
        output.emplace(u"make_hermitian", MEXEntryPointID::MakeHermitian);
        output.emplace(u"make_symmetric", MEXEntryPointID::MakeSymmetric);
        output.emplace(u"moment_matrix", MEXEntryPointID::MomentMatrix);
        output.emplace(u"new_matrix_system", MEXEntryPointID::NewMatrixSystem);
        output.emplace(u"release", MEXEntryPointID::Release);
        output.emplace(u"symbol_table", MEXEntryPointID::SymbolTable);
        output.emplace(u"version", MEXEntryPointID::Version);
        return output;
    }

    /**
     * Return the ID of the entry point for a given string representation.
     * @param str The string to compare
     * @return The ID of the entry point (if any match).
     */
    inline MEXEntryPointID which_entrypoint(const std::basic_string<char16_t>& str) {
        static const auto the_map = make_str_to_entrypoint_map();
        auto iter = the_map.find(str);
        if (iter == the_map.cend()) {
            return MEXEntryPointID::Unknown;
        }
        return iter->second;
    }

    class MexFunction;

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