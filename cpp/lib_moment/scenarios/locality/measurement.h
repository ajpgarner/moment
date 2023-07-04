/**
 * measurement.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "integer_types.h"
#include "party_measurement_index.h"

#include <cassert>
#include <string>

namespace Moment::Locality {

    class Measurement {
    public:
        /** Name of measurement */
        std::string name{};

        /** Number of measurement outcomes */
        oper_name_t num_outcomes = 0;

    private:
        /** Info about measurement, with respect to wider context of parties and other measurements */
        PMIndex index;

        /** Offset of measurement's operators within the context of owning party */
        oper_name_t party_offset = 0;

    public:
        constexpr Measurement() = default;

        constexpr Measurement(const Measurement &) = default;

        constexpr Measurement(Measurement &&) = default;

        constexpr Measurement(std::string name, oper_name_t outcomes) noexcept
                : name{std::move(name)}, num_outcomes{outcomes}  {
            assert(outcomes >= 1);
        }

        [[nodiscard]] constexpr oper_name_t num_operators() const noexcept {
            return this->num_outcomes - 1;
        }

        [[nodiscard]] constexpr const PMIndex& Index() const noexcept { return this->index; }

        friend class Party;
    };
}