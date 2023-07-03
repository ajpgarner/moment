/**
 * tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <span>
#include <stdexcept>
#include <string>
#include <vector>


namespace Moment {

    namespace errors {
        class bad_tensor_index : public std::invalid_argument {
        public:
            explicit bad_tensor_index(const std::string& what) noexcept : std::invalid_argument(what) { }
        };
    }

    class Tensor {
    public:
        using Index = std::vector<size_t>;
        using IndexView = std::span<const size_t>;

    public:
        const std::vector<size_t> Dimensions;

        const std::vector<size_t> Strides;

        const size_t DimensionCount;

        const size_t ElementCount;

    public:
        /**
         * Construct tensor of supplied dimensions.
         * @param dimensions
         */
        explicit Tensor(std::vector<size_t>&& dimensions);

        /**
         * Check that an index has the right number of elements, and is in range.
         * @throws bad_tensor_index if indices are invalid
         */
        void validate_index(IndexView indices) const;

        /**
         * Converts an index to its numerical offset within the tensor.
         */
        size_t index_to_offset(IndexView indices) const {
            this->validate_index(indices);
            return this->index_to_offset_no_checks(indices);
        }

    protected:
        /**
         * Converts an index to its numerical offset within the tensor.
         */
        size_t index_to_offset_no_checks(IndexView indices) const noexcept;

    };

}