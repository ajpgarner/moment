/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include "joint_measurement_index.h"

#include "symbolic/linear_combo.h"

#include <vector>
#include <stdexcept>

namespace NPATK {
    class SymbolTable;
    class ExplicitSymbolIndex;
    class JointMeasurementIterator;

    namespace errors {
        class bad_implicit_symbol : std::logic_error {
        public:
            explicit bad_implicit_symbol(const std::string& what) : std::logic_error(what) { }
        };
    }

    /** Definition of an implied symbol */
    struct PMODefinition {
        symbol_name_t symbol_id = 0;
        SymbolCombo expression{};

    public:
        constexpr PMODefinition(symbol_name_t symbol_id, SymbolCombo expr)
                : symbol_id{symbol_id}, expression(std::move(expr)) { }
    };

    /**
     * Calculate the 'missing' marginals/probabilities from the explicit form.
     */
    class ImplicitSymbols {
    public:
        const size_t MaxSequenceLength;
        const SymbolTable& symbols;
        const ExplicitSymbolIndex& esiForm;

    protected:
        std::vector<PMODefinition> tableData{};


    protected:
        ImplicitSymbols(const SymbolTable &st, const ExplicitSymbolIndex& esi, size_t max_length)
                : symbols{st}, esiForm{esi}, MaxSequenceLength{max_length} { }

    public:
        virtual ~ImplicitSymbols() = default;

        [[nodiscard]] constexpr const std::vector<PMODefinition>& Data() const noexcept {
            return this->tableData;
        }

        [[nodiscard]] virtual std::span<const PMODefinition> get(std::span<const size_t> mmtIndex) const = 0;

        [[nodiscard]] inline auto get(std::initializer_list<size_t> mmtIndex) const {
            std::vector<size_t> v{mmtIndex};
            return this->get(std::span(v.begin(), v.size()));
        }
    };
}