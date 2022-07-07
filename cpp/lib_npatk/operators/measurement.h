/**
 * measurement.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"

#include <cassert>
#include <string>

namespace NPATK {

    using mmt_name_t = int16_t;

    struct PMIndex {
    public:
        party_name_t party = 0;
        mmt_name_t mmt = 0;
        mmt_name_t global_mmt = 0;

    public:
        [[nodiscard]] constexpr uint64_t hash() const noexcept {
            return (static_cast<uint64_t>(this->party) << 48) + (static_cast<uint64_t>(this->mmt) << 32);
        }
    };

    struct PMOIndex {
    public:
        party_name_t party = 0;
        mmt_name_t mmt = 0;
        uint32_t outcome = 0;

    public:
        PMOIndex() = default;

        PMOIndex(PMIndex pm, uint32_t o) : party{pm.party}, mmt{pm.mmt}, outcome{o} {}

        PMOIndex(party_name_t p, mmt_name_t m, uint32_t o) : party{p}, mmt{m}, outcome{o} {}

        [[nodiscard]] constexpr uint64_t hash() const noexcept {
            return (static_cast<uint64_t>(this->party) << 48) + (static_cast<uint64_t>(this->mmt) << 32)
                   + static_cast<uint64_t>(this->outcome);
        }
    };

    struct Measurement {
    public:
        /** Name of measurement */
        std::string name{};

        /** Number of measurement outcomes */
        size_t num_outcomes = 0;

        /** True if measurement outcomes are mutually exclusive */
        bool projective = true;

        /** True if measurement outcomes sum to unity */
        bool complete = true;

    protected:
        /** Info about measurement, with respect to wider context of parties and other measurements */
        PMIndex index;

        /** Offset of measurement's operators within the context of owning party */
        size_t offset = 0;

    public:
        constexpr Measurement() = default;

        constexpr Measurement(const Measurement &) = default;

        constexpr Measurement(Measurement &&) = default;

        constexpr Measurement(std::string name, size_t outcomes,
                              bool projective = true,
                              bool complete = true) noexcept
                : name{std::move(name)}, num_outcomes{outcomes},
                  projective{projective}, complete{complete} {
            assert(outcomes >= 1);
        }

        [[nodiscard]] constexpr size_t get_offset() const noexcept { return this->offset; }

        [[nodiscard]] constexpr size_t num_operators() const noexcept {
            return this->num_outcomes - (this->complete ? 1 : 0);
        }

        [[nodiscard]] constexpr const PMIndex& Index() const noexcept { return this->index; }

        friend class Party;
    };
}