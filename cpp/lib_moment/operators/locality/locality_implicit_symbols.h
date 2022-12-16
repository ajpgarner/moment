/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/common/implicit_symbols.h"

#include "locality_context.h"
#include "measurement.h"

namespace Moment {

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

    private:
        size_t generateLevelZero(size_t& index_cursor);
        size_t generateLevelOne(size_t& index_cursor);
        size_t generateMoreLevels(size_t level, size_t& index_cursor);
        size_t generateFromCurrentStack(const JointMeasurementIterator& stack, size_t& index_cursor);
    };
}