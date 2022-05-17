/**
 * function_list.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include <map>

namespace NPATK::mex::functions {
    enum class MEXEntryPointID : int {
        Unknown = 0,
        Version = 1,
        MakeSymmetric = 2,
        MakeHermitian = 3
    };

    /**
     * Register association between strings and entry point IDs.
     * @return
     */
    inline std::map<std::basic_string<char16_t>, MEXEntryPointID> make_str_to_entrypoint_map() {
        std::map<std::basic_string<char16_t>, MEXEntryPointID> output;
        output.emplace(u"version", MEXEntryPointID::Version);
        output.emplace(u"make_symmetric", MEXEntryPointID::MakeSymmetric);
        output.emplace(u"make_hermitian", MEXEntryPointID::MakeHermitian);
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
}