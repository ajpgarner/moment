/**
 * implicit_symbols.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "joint_measurement_index.h"

#include "symbolic/linear_combo.h"

#include <cassert>

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>


namespace Moment {
    class SymbolTable;
    class ExplicitSymbolIndex;

    namespace errors {
        class bad_implicit_symbol : public std::logic_error {
        public:
            explicit bad_implicit_symbol(const std::string& what) : std::logic_error(what) { }
        };

        /** Errors when attempting to convert probability distribution with implicit symbols to explicit values */
        class implicit_to_explicit_error : public std::runtime_error {
        public:
            explicit implicit_to_explicit_error(const std::string& what) : std::runtime_error(what) { }
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

        [[nodiscard]] auto begin() const { return this->tableData.begin(); }
        [[nodiscard]] auto end() const {  return this->tableData.end(); }


        [[nodiscard]] constexpr const std::vector<PMODefinition>& Data() const noexcept {
            return this->tableData;
        }

        [[nodiscard]] virtual std::span<const PMODefinition> get(std::span<const size_t> mmtIndex) const = 0;

        [[nodiscard]] inline auto get(std::initializer_list<size_t> mmtIndex) const {
            std::vector<size_t> v{mmtIndex};
            return this->get(std::span(v.begin(), v.size()));
        }

        /**
       * Convert a full probability distribution to a list of explicit symbol assignments for the same distribution.
       * @param symbol_definitions A block containing the implicit symbol definitions
       * @param input_values The full probability distribution including implicit symbols (size must match above.)
       * @return Vector of pairs, first being symbol ID, second being the calculated value, defining input explicitly.
       */
        static std::map<symbol_name_t, double>
        implicit_to_explicit(std::span<const size_t> outcomes_per_mmt,
                             std::span<const PMODefinition> symbol_definitions,
                             std::span<const double> input_values);
    };
}