/**
 * multi_mmt_iterator.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol.h"
#include "operator_sequence.h"
#include "party.h"
#include "utilities/recursive_index.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <span>
#include <vector>

namespace NPATK {

/**
 * Iterate over combinations of measurements, from specified parties
 */
struct MultiMmtIterator {
public:
    using mmt_iter_t = std::vector<Measurement>::const_iterator;

    class OpSeqIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:

        const MultiMmtIterator * mmIter;
        MultiDimensionalIndexIterator indexIter;

    public:
        explicit OpSeqIterator(const MultiMmtIterator& mmIter, bool end = false);

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

    class OutcomeIndexIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = MultiDimensionalIndexIterator::value_type;

    private:
        const MultiMmtIterator * mmIter;
        MultiDimensionalIndexIterator indexIter;
        std::vector<bool> is_implicit;
        size_t num_implicit = 0;
        size_t operNumber = 0;

    public:
        explicit OutcomeIndexIterator(const MultiMmtIterator& mmIter, bool end = false);

        OutcomeIndexIterator& operator++() noexcept;

        [[nodiscard]] OutcomeIndexIterator operator++(int) & noexcept {
            OutcomeIndexIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] inline auto operator*() const noexcept {
            return *this->indexIter;
        }

        [[nodiscard]] inline auto operator[](size_t index) const noexcept {
            return this->indexIter.operator[](index);
        }

        /**
         * Vector of bools, indicating which indices do not correspond to explicitly defined operators.
         */
        [[nodiscard]] const auto& implicit() const noexcept {
            return this->is_implicit;
        }

        /**
         * If operator is explicitly defined, get the operator's index.
         */
        size_t explicit_op_index() const noexcept {
            assert(this->num_implicit == 0);
            return this->operNumber;
        }

        /**
         * The subset of measurement/outcomes that do not correspond to an explicitly defined operator in a CG table.
         * @param implicit Set to true to return implicitly defined mmt/outcome pairs; false for the complement of this.
         * @return Pair, vector of pairs, first: iterators to mmts, second: outcome numbers
         */
        [[nodiscard]] std::vector<std::pair<mmt_iter_t, size_t>> implicit_indices(bool implicit = true) const;

        /**
         * The subset of measurement/outcomes that correspond to an explicitly defined operator in a CG table.
         * Complement of implicit_indices(true).
         * @return Pair, vector of pairs, first: iterators to mmts, second: outcome numbers
         */
        [[nodiscard]] inline std::vector<std::pair<mmt_iter_t, size_t>> explicit_indices() const {
            return implicit_indices(false);
        }

        /**
         * Number of indices that are "out of bounds" in the CG form.
         */
        [[nodiscard]] const auto& implicit_count() const noexcept {
            return this->num_implicit;
        }

        [[nodiscard]] bool operator==(const OutcomeIndexIterator& rhs) const noexcept {
            assert(this->mmIter == rhs.mmIter);
            return this->indexIter == rhs.indexIter;
        }

        [[nodiscard]] bool operator!=(const OutcomeIndexIterator& rhs) const noexcept {
            assert(this->mmIter == rhs.mmIter);
            return this->indexIter != rhs.indexIter;
        }

    private:
        void check_implicit();
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

    const Context * contextPtr = nullptr;
    bool is_done = false;

public:
    explicit MultiMmtIterator(const Context& context, party_list_t&& list);

    inline MultiMmtIterator& operator++() noexcept {
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

    /** Size of index vectors */
    [[nodiscard]] size_t dimension() const noexcept {
        return this->mmt_indices.size();
    }

    [[nodiscard]] size_t count_outcomes() const noexcept;

    [[nodiscard]] size_t count_operators() const noexcept;

    [[nodiscard]] inline auto begin_operators() const noexcept {
        return OpSeqIterator{*this};
    }

    [[nodiscard]] inline auto end_operators() const noexcept {
        return OpSeqIterator{*this, true};
    }

    [[nodiscard]] inline auto begin_outcomes() const noexcept {
        return OutcomeIndexIterator{*this};
    }

    [[nodiscard]] inline auto end_outcomes() const noexcept {
        return OutcomeIndexIterator{*this, true};
    }
};

}