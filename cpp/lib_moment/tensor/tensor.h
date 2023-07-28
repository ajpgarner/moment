/**
 * multi_dimensional_object.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "tensor_errors.h"
#include "multi_dimensional_index_iterator.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <span>
#include <vector>

namespace Moment {

    template<typename index_elem_t, typename index_storage_t, bool LastIndexMajor>
    class StrideCalculator {
    public:
        [[nodiscard]] constexpr index_storage_t operator()(const index_storage_t &dimensions) const {
            index_storage_t strides;
            if constexpr (LastIndexMajor) {
                strides.reserve(dimensions.size());
                std::exclusive_scan(dimensions.begin(), dimensions.end(),
                                    std::back_inserter(strides),
                                    static_cast<index_elem_t>(1), std::multiplies{});
            } else {
                strides.assign(dimensions.size(), 1);
                std::exclusive_scan(dimensions.rbegin(), dimensions.rend(),
                                    strides.rbegin(),
                                    static_cast<index_elem_t>(1), std::multiplies{});
            }
            return strides;
        }
    };

    template<typename index_elem_t, bool LastIndexMajor, size_t N>
    class StrideCalculator<index_elem_t, std::array<index_elem_t, N>, LastIndexMajor> {
    public:

        [[nodiscard]] constexpr std::array<index_elem_t, N>
         operator()(const std::array<index_elem_t, N>& dimensions) const {
            std::array<index_elem_t, N> strides;
            if constexpr (LastIndexMajor) {
                std::exclusive_scan(dimensions.begin(), dimensions.end(),
                                    strides.begin(),
                                    static_cast<index_elem_t>(1), std::multiplies{});
            } else {
                std::exclusive_scan(dimensions.rbegin(), dimensions.rend(),
                                    strides.rbegin(),
                                    static_cast<index_elem_t>(1), std::multiplies{});
            }
            return strides;
        }
    };


    template<typename index_elem_t, typename index_storage_t, bool LastIndexMajor>
    class IndexToOffsetCalculator {
    public:
        const index_storage_t& Dimensions;
        const size_t DimensionCount;
        const index_storage_t& Strides;

        constexpr IndexToOffsetCalculator(const index_storage_t& dimensions, const index_storage_t& strides)
            : Dimensions{dimensions},
              DimensionCount{dimensions.size()},
              Strides{strides} { }

        [[nodiscard]] constexpr index_storage_t operator()(index_elem_t offset) const {
            index_storage_t output;
                output.reserve(this->DimensionCount);
                if constexpr (LastIndexMajor) {
                    for (size_t n = 0; n < this->DimensionCount; ++n) {
                        output.emplace_back(offset % this->Dimensions[n]);
                        offset /= this->Dimensions[n];
                    }
                } else {
                    for (size_t n = 0; n < this->DimensionCount; ++n) {
                        output.emplace_back(offset / this->Strides[n]);
                        offset %= this->Strides[n];
                    }
                }
                return output;
        }
    };

    template<typename index_elem_t, bool LastIndexMajor, size_t N>
    class IndexToOffsetCalculator<index_elem_t, std::array<index_elem_t, N>, LastIndexMajor> {

    public:
        const std::array<index_elem_t, N>& Dimensions;
        const std::array<index_elem_t, N>& Strides;

        constexpr IndexToOffsetCalculator(const std::array<index_elem_t, N>& dimensions,
                                const std::array<index_elem_t, N>& strides)
                : Dimensions{dimensions}, Strides{strides} { }


        [[nodiscard]] constexpr std::array<index_elem_t, N>
        operator()(index_elem_t offset) const {

            std::array<index_elem_t, N> output;
            if constexpr (LastIndexMajor) {
                for (size_t n = 0; n < N; ++n) {
                    output[n] = offset % this->Dimensions[n];
                    offset /= this->Dimensions[n];
                }
            } else {
                for (size_t n = 0; n < N; ++n) {
                    output[n] = offset / this->Strides[n];
                    offset %= this->Strides[n];
                }
            }
            return output;
        }
    };


    /**
     * An object with indexed dimensions, which can be converted to a numerical offset.
     * @tparam index_elem_t The integral type underlining the index.
     * @tparam index_storage_t The storage of multi-dimensional indices.
     * @tparam index_view_t A view to a multi-dimensional index.
     * @tparam last_index_major True if the offset is last-index-major (e.g. col-major in two dimensions).
     */
    template<std::integral index_elem_t = size_t,
            typename index_storage_t = std::vector<size_t>,
            typename index_view_t = std::span<const size_t>,
            bool last_index_major = true>
    struct Tensor {
    public:
        using IndexElement = index_elem_t;
        using Index = index_storage_t;
        using IndexView = index_view_t;
        using IndexIterator = MultiDimensionalIndexIterator<last_index_major, Index>;

        /** True if storage order is last-index-major (e.g. col-major in two dimensions). */
        constexpr static const bool LastIndexMajor = last_index_major;

        /** The dimensions of the object. */
        const Index Dimensions;

        /** The distance in offset represented by each dimension. */
        const Index Strides;

        /** The number of dimensions in the object */
        const IndexElement DimensionCount;

        /** The number of unique elements represented by the object. */
        const IndexElement ElementCount;

        /**
         * Constructs a new multi-dimensional tensor object.
         * @param dimensions A list of sizes per dimension.
         */
        explicit Tensor(Index dimensions)
            : Dimensions{std::move(dimensions)},
              Strides{StrideCalculator<IndexElement , Index, last_index_major>{}(Dimensions)},
              DimensionCount{static_cast<IndexElement>(Dimensions.size())},
              ElementCount{calculateNumberOfElements(Dimensions)} { }

        /**
         * Converts an index to its numerical offset within the tensor.
         */
        [[nodiscard]] inline constexpr IndexElement index_to_offset_no_checks(IndexView indices) const noexcept {
            return std::inner_product(indices.begin(), indices.end(),
                                      this->Strides.begin(), static_cast<IndexElement>(0));
        }

        /**
         * Check an index is in bounds, then convert to numerical offset.
         */
        [[nodiscard]] inline IndexElement index_to_offset(IndexView indices) const {
            this->validate_index(indices);
            return this->index_to_offset_no_checks(indices);
        }


        /**
         * Checks that an index is in range.
         * @throws bad_tensor_index if index is invalid.
         */
         void validate_index(IndexView index) const {
             if (index.size() != this->DimensionCount) {
                 throw Moment::errors::bad_tensor_index::bad_dimension_count(index.size(), this->DimensionCount);
             }

             if constexpr (std::is_signed_v<IndexElement>) {
                 for (size_t d = 0; d < index.size(); ++d) {
                    if ((index[d] < 0) || (index[d] >= this->Dimensions[d])) {
                        throw Moment::errors::bad_tensor_index::index_out_of_range(d, index[d], this->Dimensions[d]);
                    }
                 }
             } else {
                 for (size_t d = 0; d < index.size(); ++d) {
                     if (index[d] >= this->Dimensions[d]) {
                         throw Moment::errors::bad_tensor_index::index_out_of_range(d, index[d], this->Dimensions[d]);
                     }
                 }
             }
         }

        /**
         * Checks that an index is in inclusive range (i.e. allow 'past-the-end' elements for dimensions.)
         * @throws bad_tensor_index if index is invalid.
         */
         void validate_index_inclusive(IndexView index) const {
             if (index.size() != this->DimensionCount) {
                 throw Moment::errors::bad_tensor_index::bad_dimension_count(index.size(), this->DimensionCount);
             }

             if constexpr (std::is_signed_v<IndexElement>) {
                 for (size_t d = 0; d < index.size(); ++d) {
                    if ((index[d] < 0) || (index[d] > this->Dimensions[d])) {
                        throw Moment::errors::bad_tensor_index::index_out_of_range(d, index[d], this->Dimensions[d]);
                    }
                 }
             } else {
                 for (size_t d = 0; d < index.size(); ++d) {
                     if (index[d] > this->Dimensions[d]) {
                         throw Moment::errors::bad_tensor_index::index_out_of_range(d, index[d], this->Dimensions[d]);
                     }
                 }
             }
         }

        /**
         * Converts a numerical offset to its index within the tensor.
         */
        [[nodiscard]] inline constexpr Index offset_to_index_no_checks(IndexElement offset) const noexcept {
            IndexToOffsetCalculator<IndexElement, Index, last_index_major> calculator{this->Dimensions, this->Strides};
            return calculator(offset);
        }

        /**
         * Checks that an offset is in range.
         * @throws bad_tensor_index if offset is invalid.
         */
        void validate_offset(IndexElement offset) const {
            if constexpr (std::is_signed_v<IndexElement>) {
                if ((offset < 0) || (offset >= this->ElementCount)) {
                    throw Moment::errors::bad_tensor_index::offset_out_of_range(offset, this->ElementCount);
                }
            } else {
                if (offset >= this->ElementCount) {
                    throw Moment::errors::bad_tensor_index::offset_out_of_range(offset, this->ElementCount);
                }
            }
        }

        /**
         * Check that a pair of indices has the right number of elements, are in bounds, and refer to a positive range.
         */
        void validate_range(IndexView min, IndexView max) const {
            this->validate_index(min);
            this->validate_index_inclusive(max);
            for (size_t d = 0; d < static_cast<size_t>(this->DimensionCount); ++d) {
                if (min[d] > max[d]) {
                    throw errors::bad_tensor_index::wrong_order(d, min[d], max[d]);
                }
            }
        }

        /**
         * Check an index is in bounds, then convert to numerical offset.
         */
        [[nodiscard]] inline Index offset_to_index(IndexElement offset) const {
            this->validate_offset(offset);
            return this->offset_to_index_no_checks(offset);
        }

    private:

        /**
         * Calculates the number of elements from the dimensions.
         */
        [[nodiscard]] static constexpr IndexElement calculateNumberOfElements(const IndexView dimensions) {
            if (dimensions.empty()) {
                return 0;
            }
            return std::reduce(dimensions.begin(), dimensions.end(), static_cast<IndexElement>(1), std::multiplies{});
        }

    };
}
