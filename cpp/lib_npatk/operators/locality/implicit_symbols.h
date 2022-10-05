/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "locality_context.h"
#include "measurement.h"
#include "joint_measurement_index.h"

#include "symbolic/linear_combo.h"

#include "integer_types.h"

#include <vector>
#include <stdexcept>

namespace NPATK {

    class LocalityMatrixSystem;
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
     * Calculate the 'missing' marginals/probabilities from the Gisin form.
     */
    class ImplicitSymbols {
    public:
        const LocalityContext& context;
    public:
        const size_t MaxSequenceLength;

        const SymbolTable& symbols;
        const ExplicitSymbolIndex& esiForm;

    private:
        std::vector<PMODefinition> tableData{};
        JointMeasurementIndex indices;

    public:
        explicit ImplicitSymbols(const LocalityMatrixSystem& ms);

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


        [[nodiscard]] const PMODefinition& get(std::span<const PMOIndex> lookup_indices) const;

        template<typename functor_t>
        void visit(functor_t& visitor) const {
            auto visitor_wrapper = [&](const std::pair<size_t, size_t>& tableRange,
                                       std::span<const size_t> global_indices) {
                std::span<const PMODefinition> pmoSpan{tableData.cbegin() + static_cast<ptrdiff_t>(tableRange.first),
                                                       static_cast<size_t>(tableRange.second - tableRange.first)};
                std::vector<PMIndex> converted;
                converted.reserve(global_indices.size());
                for (auto global_index : global_indices) {
                    converted.push_back(this->context.global_index_to_PM(global_index));
                }
                visitor(pmoSpan, std::span<const PMIndex>(converted.cbegin(), converted.size()));
            };

            this->indices.visit(visitor_wrapper);
        }

    private:
        size_t generateLevelZero(size_t& index_cursor);
        size_t generateLevelOne(size_t& index_cursor);
        size_t generateMoreLevels(size_t level, size_t& index_cursor);
        size_t generateFromCurrentStack(const JointMeasurementIterator& stack, size_t& index_cursor);
    };
}