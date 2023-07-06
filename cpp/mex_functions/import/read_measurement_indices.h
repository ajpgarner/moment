/**
 * read_measurement_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/locality/party_measurement_index.h"
#include "scenarios/inflation/observable_variant_index.h"

#include "MatlabDataArray.hpp"

#include <span>
#include <vector>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    namespace Locality {
        class LocalityContext;
    }
    namespace Inflation {
        class InflationContext;
    }
}

namespace Moment::mex {

    /**
     * Pair of indices, without context.
     */
    struct RawIndexPair {
        size_t first, second;

        constexpr RawIndexPair() = default;
        constexpr RawIndexPair(size_t first, size_t second, size_t third = 0) : first{first}, second{second} { }

        /**
         * Read a Nx2 matlab array into a vector of RawIndexPairs.
         * @param matlabEngine The matlab engine.
         * @param input A Nx2 matlab array (numeric, or strings parsable as numbers).
         * @return A vector of RawIndexPairs.
         */
        static std::vector<RawIndexPair> read_list(matlab::engine::MATLABEngine& matlabEngine,
                                                   const matlab::data::Array& input);
    };

    /**
     * Triplet of indices, without context.
     */
    struct RawIndexTriplet {
        size_t first, second, third;

        constexpr RawIndexTriplet() = default;
        constexpr RawIndexTriplet(size_t first, size_t second, size_t third) : first{first}, second{second}, third{third} { }

        /**
         * Read a Nx3 matlab array into a vector of RawIndexTriplets.
         * @param matlabEngine The matlab engine.
         * @param input A Nx3 matlab array (numeric, or strings parsable as numbers).
         * @return A vector of RawIndexTriplets.
         */
        static std::vector<RawIndexTriplet> read_list(matlab::engine::MATLABEngine& matlabEngine,
                                                      const matlab::data::Array& input);
    };


    [[nodiscard]] std::pair<std::vector<RawIndexPair>, std::vector<RawIndexTriplet>>
    read_pairs_and_triplets(matlab::engine::MATLABEngine& engine, const matlab::data::Array& first_array);

    [[nodiscard]] std::pair<std::vector<RawIndexPair>, std::vector<RawIndexTriplet>>
    read_pairs_and_triplets(matlab::engine::MATLABEngine& engine, const matlab::data::Array& first_array,
                                                                  const matlab::data::Array& second_array);

    /**
     * Reads raw indices into Party/Measurement/(Outcome) indices, with bounds checks.
     */
  class PMConvertor {
  public:
      const Locality::LocalityContext& context;
      matlab::engine::MATLABEngine& matlabEngine;

      /** Set to true to allow final outcome (e.g. for probability tensor); false to disallow (e.g. for C-G tensor).*/
        bool inclusive = true;

        /**
         * Reads raw indices into Party/Measurement/(Outcome) indices, with bounds checks.
         * @param matlabEngine The matlab engine.
         * @param context The locality context.
         * @param inclusive Allow final outcome (e.g. for probability tensor); false to disallow (e.g. for C-G tensor).
         */
        PMConvertor(matlab::engine::MATLABEngine& matlabEngine,
                    const Locality::LocalityContext& context,
                    const bool inclusive) : matlabEngine{matlabEngine}, context{context}, inclusive{inclusive} { }

        Locality::PMIndex read_pm_index(const RawIndexPair& pair);

        Locality::PMOIndex read_pmo_index(const RawIndexTriplet& triplet);

        std::vector<Locality::PMIndex> read_pm_index_list(std::span<const RawIndexPair>);

        std::vector<Locality::PMOIndex> read_pmo_index_list(std::span<const RawIndexTriplet>);
    };


    /**
     * Reads raw indices into Observable/Variant/(Outcome) indices, with bounds checks.
     */
    class OVConvertor {
    public:
        const Inflation::InflationContext& context;
        matlab::engine::MATLABEngine& matlabEngine;

        /** Set to true to allow final outcome (e.g. for probability tensor); false to disallow (e.g. for C-G tensor).*/
        bool inclusive = true;

        /**
         * Reads raw indices into Observable/Variant/(Outcome) indices, with bounds checks.
         * @param matlabEngine The matlab engine.
         * @param context The inflation context.
         * @param inclusive Allow final outcome (e.g. for probability tensor); false to disallow (e.g. for C-G tensor).
         */
        OVConvertor(matlab::engine::MATLABEngine& matlabEngine,
                    const Inflation::InflationContext& context,
                    const bool inclusive) : matlabEngine{matlabEngine}, context{context}, inclusive{inclusive} { }

        /**
         * Convert raw pair into Observable-Variant index.
         */
        Inflation::OVIndex read_ov_index(const RawIndexPair& pair);

        /**
         * Convert raw triplet into Observable-Variant-Outcome index.
         */
        Inflation::OVOIndex read_ovo_index(const RawIndexTriplet& triplet);

        /**
         * Convert span of raw pairs into vector of Observable-Variant indices.
         */
        std::vector<Inflation::OVIndex> read_ov_index_list(std::span<const RawIndexPair>);

        /**
         * Convert span of raw triplet into vector of Observable-Variant-Outcome indices.
         */
        std::vector<Inflation::OVOIndex> read_ovo_index_list(std::span<const RawIndexTriplet>);
    };

}