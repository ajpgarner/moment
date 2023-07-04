/**
 * party_measurement_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include <limits>

namespace Moment::Locality {
    class LocalityContext;

    struct PMIndex {
    public:
        party_name_t party;
        mmt_name_t mmt;
        size_t global_mmt;

    public:
        constexpr PMIndex() = default;

        constexpr PMIndex(party_name_t party, mmt_name_t mmt, size_t global_mmt = std::numeric_limits<size_t>::max())
                : party{party}, mmt{mmt}, global_mmt{global_mmt} { }

        PMIndex(const LocalityContext& context, party_name_t party, mmt_name_t mmt);
    };

    struct PMOIndex : public PMIndex {
    public:
        uint32_t outcome = 0;

    public:
        constexpr PMOIndex() = default;

        constexpr PMOIndex(PMIndex pm, uint32_t o) : PMIndex{pm}, outcome{o} {}

        constexpr PMOIndex(party_name_t p, mmt_name_t m, uint32_t o) : PMIndex{p, m}, outcome{o} {}

        PMOIndex(const LocalityContext& context, party_name_t p, mmt_name_t m, uint32_t o)
            : PMIndex{context, p, m}, outcome{o} {}


    };
}