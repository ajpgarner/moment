/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "moment_matrix.h"
#include "symbolic/linear_combo.h"
#include "joint_measurement_index.h"

#include <vector>
#include <stdexcept>

namespace NPATK {

    class JointMeasurementIterator;

    namespace errors {
        class bad_implicit_symbol : std::logic_error {
        public:
            bad_implicit_symbol(const std::string& what) : std::logic_error(what) { }
        };
    }

    /**
     * Calculate the 'missing' marginals/probabilities from the Gisin form.
     */
    class ImplicitSymbols {
    public:
        using SymbolCombo = LinearCombo<symbol_name_t, double>;

        struct PMODefinition {
            symbol_name_t symbol_id = 0;
            SymbolCombo expression{};

        public:
            constexpr PMODefinition(symbol_name_t symbol_id, SymbolCombo expr)
                : symbol_id{symbol_id}, expression(std::move(expr)) { }
        };

    public:
        const size_t MaxSequenceLength;
        const MomentMatrix& momentMatrix;
        const CollinsGisinForm& cgForm;
        const Context& context;

    private:
        std::vector<PMODefinition> tableData{};
        JointMeasurementIndex indices;

    public:
        explicit ImplicitSymbols(const MomentMatrix& mm);

        [[nodiscard]] constexpr const std::vector<PMODefinition>& Data() const noexcept {
            return this->tableData;
        }

        [[nodiscard]] constexpr const JointMeasurementIndex& Indices() const noexcept {
            return this->indices;
        }

        [[nodiscard]] std::span<const PMODefinition> get(std::span<const size_t> mmtIndex) const;

        [[nodiscard]] inline auto get(std::initializer_list<size_t> mmtIndex) const {
            std::vector<size_t> v{mmtIndex};
            return this->get({v.begin(), v.size()});
        }

    private:
        size_t generateLevelZero(size_t& index_cursor);
        size_t generateLevelOne(size_t& index_cursor);
        size_t generateMoreLevels(size_t level, size_t& index_cursor);
        size_t generateFromCurrentStack(const JointMeasurementIterator& stack, size_t& index_cursor);
    };
}