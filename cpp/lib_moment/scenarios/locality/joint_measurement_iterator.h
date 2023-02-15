/**
 * joint_measurement_iterator.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 *
 */
#pragma once

#include "party.h"

#include "../operator_sequence.h"
#include "../outcome_index_iterator.h"

#include "utilities/multi_dimensional_index_iterator.h"

#include <span>
#include <vector>

namespace Moment::Locality {

    class LocalityContext;
    class JointMeasurementIterator;


/**
 * Iterate over combinations of measurements, from specified parties
 */
struct JointMeasurementIterator {
public:
    using mmt_iter_t = std::vector<Measurement>::const_iterator;

    class OpSeqIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:

        const JointMeasurementIterator * mmIter;
        MultiDimensionalIndexIterator<false> indexIter;

    public:
        explicit OpSeqIterator(const JointMeasurementIterator& mmIter, bool end = false);

        OpSeqIterator& operator++() noexcept {
            ++indexIter;
            return *this;
        }

        [[nodiscard]] OpSeqIterator operator++(int) & noexcept {
            OpSeqIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] OperatorSequence operator*() const;

        [[nodiscard]] bool operator==(const OpSeqIterator& rhs)  const noexcept {
            assert(this->mmIter == rhs.mmIter);
            return this->indexIter == rhs.indexIter;
        }

        [[nodiscard]] bool operator!=(const OpSeqIterator& rhs)  const noexcept {
            assert(this->mmIter == rhs.mmIter);
            return this->indexIter != rhs.indexIter;
        }
    };

    static_assert(std::input_iterator<OpSeqIterator>);

public:
    using party_list_t = std::vector<const Party*>;

private:
    party_list_t party_list;

    /** Indices of measurements, relative to Party */
    std::vector<size_t> mmt_indices;

    /** Global indices of measurements, relative to Context */
    std::vector<size_t> global_mmt_indices;

    /** One iterator through measurements per Party. */
    std::vector<mmt_iter_t> mmt_iters;

    const LocalityContext * contextPtr = nullptr;

    bool is_done = false;

public:
    explicit JointMeasurementIterator(const LocalityContext& context, party_list_t&& list);

    inline JointMeasurementIterator& operator++() noexcept {
        next();
        return *this;
    }

    void next() noexcept;

    [[nodiscard]] constexpr bool done() const noexcept { return this->is_done; }

    [[nodiscard]] std::span<const size_t> indices() const noexcept {
        return {this->mmt_indices.begin(), this->mmt_indices.size()};
    }

    [[nodiscard]] std::span<const size_t> global_indices() const noexcept {
        return {this->global_mmt_indices.begin(), this->global_mmt_indices.size()};
    }

    [[nodiscard]] auto iters() const noexcept {
        return std::span{this->mmt_iters.cbegin(), this->mmt_iters.size()};
    }

    /** Number of indices (i.e. dimension of index vector) */
    [[nodiscard]] size_t count_indices() const noexcept {
        return this->mmt_indices.size();
    }

    /** Total number of outcomes associated with joint measurement. */
    [[nodiscard]] size_t count_outcomes() const noexcept;

    /** Total number of explicitly defined operators associated with joint measurement. */
    [[nodiscard]] size_t count_operators() const noexcept;

    [[nodiscard]] inline auto begin_operators() const noexcept {
        return OpSeqIterator{*this};
    }

    [[nodiscard]] inline auto end_operators() const noexcept {
        return OpSeqIterator{*this, true};
    }

    [[nodiscard]] OutcomeIndexIterator begin_outcomes() const noexcept;

    [[nodiscard]] OutcomeIndexIterator end_outcomes() const noexcept;
};

}