/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "scenarios/implicit_symbols.h"

#include "locality_context.h"
#include "measurement.h"

namespace Moment::Locality {
    class JointMeasurementIterator;
    class LocalityMatrixSystem;

    /**
     * Calculate the 'missing' marginals/probabilities from the explicit form.
     */
    class LocalityImplicitSymbols : public ImplicitSymbols {
    public:
        const LocalityContext& context;

    private:
        JointMeasurementIndex indices;

    public:
        explicit LocalityImplicitSymbols(const LocalityMatrixSystem& ms);

        using ImplicitSymbols::get;

        [[nodiscard]] std::span<const PMODefinition> get(std::span<const size_t> mmtIndex) const override;

        [[nodiscard]] std::span<const PMODefinition> get(std::span<const PMIndex> mmtIndex) const;

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


        [[nodiscard]] constexpr const JointMeasurementIndex& Indices() const noexcept {
            return this->indices;
        }

        using ImplicitSymbols::implicit_to_explicit;

        /**
         * Convert a full probability distribution over supplied (joint) observables to a list of explicit symbol
         * assignments imposing the same distribution.
         * @param mmtIndices The indices of the parties/measurements to use
         * @param input_values The full probability distribution including implicit symbols
         * @return Vector of pairs, first being symbol ID, second being the calculated value, defining input explicitly.
         */
        std::map<symbol_name_t, double>
        implicit_to_explicit(std::span<const PMIndex> mmtIndices, std::span<const double> input_values) const;

    private:
        size_t generateLevelZero(size_t& index_cursor);
        size_t generateLevelOne(size_t& index_cursor);
        size_t generateMoreLevels(size_t level, size_t& index_cursor);
        size_t generateFromCurrentStack(const JointMeasurementIterator& stack, size_t& index_cursor);
    };
}