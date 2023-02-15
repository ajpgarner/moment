/**
 * variable_chunk_range.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>
#include <cstdlib>

#include <iterator>
#include <span>
#include <vector>

namespace Moment {

    /**
     * Non-owning span for data split into variable-sized chunks.
     * @tparam data_t The data stored in the container
     * @tparam index_t The index type
     */
    template<typename data_t, typename index_t = size_t>
    class VariableChunkRange {
    public:
        /**
         * Iterate one chunk at a time, over data.
         */
        class VariableChunkIter {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = typename std::make_signed<index_t>::type;
            using value_type = typename std::span<data_t>;

        private:
            std::span<const data_t> data;
            std::span<const index_t> indices;

            index_t index = 0;

        private:
            VariableChunkIter(std::span<const data_t> the_data, std::span<const index_t> the_indices)
                : data{the_data}, indices{the_indices}, index{static_cast<index_t>(0)} {

            }

            VariableChunkIter(std::span<const data_t> the_data, std::span<const index_t> the_indices, bool) :
                data{the_data}, indices{the_indices}, index{static_cast<index_t>(the_indices.size())} {
            }

        public:
            VariableChunkIter() = default;

            bool operator==(const VariableChunkIter& rhs) const noexcept {
                return this->index == rhs.index;
            }

            VariableChunkIter& operator++() {
                ++this->index;
                return *this;
            }

            [[nodiscard]] VariableChunkIter operator++(int) { // NOLINT(cert-dcl21-cpp)
                auto copy = *this;
                this->operator++();
                return copy;
            }

            index_t chunk_size() const noexcept {

                if ((1+this->index) < this->indices.size()) {
                    assert(this->indices[this->index] <= this->indices[this->index+1]);
                    return this->indices[1+this->index] - this->indices[this->index];
                } else if ((1+this->index) == this->indices.size()) {
                    return this->data.size() - this->indices.back();
                } else {
                    return 0;
                }
            }

            std::span<const data_t> operator*() const noexcept {
                assert(this->index < this->indices.size());

                index_t offset = this->indices[this->index];
                index_t size = this->chunk_size();
                return {this->data.begin() + offset, static_cast<size_t>(size)};
            }

            friend class VariableChunkRange;
        };

        static_assert(std::input_iterator<VariableChunkIter>);

    private:
        const std::span<const data_t> data;
        const std::span<const index_t> indices;

    public:
        /**
         * Constructs a view of data, to be iterated over in variable-sized chunks.
         * @param the_data Reference to the data to be viewed in chunks.
         * @param the_indices Indicates the starting index (inclusive) of each chunk of data.
         */
        VariableChunkRange(const std::span<const data_t> the_data, const std::span<const index_t> the_indices)
            : data{the_data}, indices{the_indices} {
        }

        auto begin() const noexcept {
            return VariableChunkIter{this->data, this->indices};
        }

        auto end() const noexcept {
            return VariableChunkIter{this->data, this->indices, true};
        }
    };




}