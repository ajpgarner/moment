/**
 * index_flattener.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "tensor/multi_dimensional_object.h"
#include "tensor/multi_dimensional_index_iterator.h"

#include "integer_types.h"

#include <iterator>
#include <vector>

namespace Moment {
    class IndexFlattener {
    public:
        using Index = std::vector<size_t>;
        using IndexView = std::span<const size_t>;
        using ObjectType = MultiDimensionalObject<size_t, Index, IndexView, true>;

    public:
        /** Object dimensions, etc. */
        ObjectType object;

        const std::vector<std::vector<size_t>> indices_per_dimensions;

        const std::vector<size_t> index_sizes;


    public:
        class FlattenedIndexIter {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = size_t;

        private:
            ObjectType::IndexIterator index_iter;

        public:
            struct end_tag { };
            const IndexFlattener* range;
        public:
            explicit FlattenedIndexIter(const IndexFlattener& range)
                : range{&range}, index_iter{range.index_sizes} { }

            FlattenedIndexIter(const IndexFlattener& range, const end_tag& /**/)
                : range{&range}, index_iter{range.index_sizes, true} { }

            [[nodiscard]] inline bool operator==(const FlattenedIndexIter& other) const noexcept {
                return this->index_iter == other.index_iter;
            }

            [[nodiscard]] inline bool operator!=(const FlattenedIndexIter& other) const noexcept {
                return this->index_iter != other.index_iter;
            }

            inline FlattenedIndexIter& operator++() {
                ++this->index_iter;
                return *this;
            }

            [[nodiscard]] inline FlattenedIndexIter operator++(int) & {
                FlattenedIndexIter copy{*this};
                ++this->index_iter;
                return copy;
            }

            /** The index of the iterator within the list of indices. */
            [[nodiscard]] const Index& index_index() const noexcept {
                return *this->index_iter;
            }

            /** The remapped index of the object. */
            [[nodiscard]] Index index() const;


            /* The flattened index. */
            [[nodiscard]] size_t operator*() const;
        };

        static_assert(std::input_iterator<FlattenedIndexIter>);

        /**
         * Constructs a flattener range.
         */
        explicit IndexFlattener(ObjectType dimensioned_object, std::vector<std::vector<size_t>> indices);

        /**
         * Constructs a flattener range.
         * @param obj_size
         * @param indices
         */
        explicit IndexFlattener(std::vector<size_t> dimensions, std::vector<std::vector<size_t>> indices) :
                IndexFlattener(ObjectType{std::move(dimensions)}, std::move(indices)) { }


        [[nodiscard]] size_t size() const noexcept;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] FlattenedIndexIter begin() const {
            return FlattenedIndexIter{*this};
        }

        [[nodiscard]] FlattenedIndexIter end() const {
            return FlattenedIndexIter{*this, FlattenedIndexIter::end_tag{}};
        }

    };
}