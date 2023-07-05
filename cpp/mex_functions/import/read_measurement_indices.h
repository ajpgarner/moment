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

    struct RawIndexPair {
        size_t first, second;

        constexpr RawIndexPair() = default;
        constexpr RawIndexPair(size_t first, size_t second, size_t third = 0) : first{first}, second{second} { }

        static std::vector<RawIndexPair> read_list(matlab::engine::MATLABEngine& matlabEngine,
                                                   const matlab::data::Array& input);
    };

    struct RawIndexTriplet {
        size_t first, second, third;

        constexpr RawIndexTriplet() = default;
        constexpr RawIndexTriplet(size_t first, size_t second, size_t third) : first{first}, second{second}, third{third} { }

        static std::vector<RawIndexTriplet> read_list(matlab::engine::MATLABEngine& matlabEngine,
                                                      const matlab::data::Array& input);
    };

    class PMConvertor {
    public:
        const Locality::LocalityContext& context;
        matlab::engine::MATLABEngine& matlabEngine;

        PMConvertor(matlab::engine::MATLABEngine& matlabEngine,
                    const Locality::LocalityContext& context) : matlabEngine{matlabEngine}, context{context} { }

        Locality::PMIndex read_pm_index(const RawIndexPair& pair);

        Locality::PMOIndex read_pmo_index(const RawIndexTriplet& triplet);

        std::vector<Locality::PMIndex> read_pm_index_list(std::span<const RawIndexPair>);

        std::vector<Locality::PMOIndex> read_pmo_index_list(std::span<const RawIndexTriplet>);
    };

    class OVConvertor {
    public:
        const Inflation::InflationContext& context;
        matlab::engine::MATLABEngine& matlabEngine;

        OVConvertor(matlab::engine::MATLABEngine& matlabEngine,
                    const Inflation::InflationContext& context) : matlabEngine{matlabEngine}, context{context} { }

        Inflation::OVIndex read_ov_index(const RawIndexPair& pair);

        Inflation::OVOIndex read_ovo_index(const RawIndexTriplet& triplet);

        std::vector<Inflation::OVIndex> read_ov_index_list(std::span<const RawIndexPair>);

        std::vector<Inflation::OVOIndex> read_ovo_index_list(std::span<const RawIndexTriplet>);
    };

}