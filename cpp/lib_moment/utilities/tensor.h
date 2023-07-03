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
        class bad_tensor : public std::runtime_error {
        public:
            explicit bad_tensor(const std::string& what) noexcept : std::runtime_error(what) { }

        public:
            static bad_tensor no_data_stored(const std::string& name);

        };

        class bad_tensor_index : public bad_tensor {
        public:
            explicit bad_tensor_index(const std::string& what) noexcept : bad_tensor(what) { }
        };
    }

    /**
     * Tensor object.
     * Uses a generalized col-major storage order (first-index major).
     */
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
         * Checks that an offset is in range.
         * @throws bad_tensor_index if offset is invalid
         */
        void validate_offset(size_t offset) const;

        /**
         * Converts an index to its numerical offset within the tensor.
         */
        [[nodiscard]] size_t index_to_offset(IndexView indices) const {
            this->validate_index(indices);
            return this->index_to_offset_no_checks(indices);
        }

       /**
         * Converts an index to its numerical offset within the tensor.
         */
        [[nodiscard]] Index offset_to_index(const size_t offset) const {
            this->validate_offset(offset);
            return this->offset_to_index_no_checks(offset);
        }

    protected:
        /**
         * Converts an index to its numerical offset within the tensor.
         */
        [[nodiscard]] size_t index_to_offset_no_checks(IndexView indices) const noexcept;

        /**
         * Converts a numerical offset to its index within the tensor.
         * Do not use this in a loop! Prefer an iterator object.
         */
        [[nodiscard]] Index offset_to_index_no_checks(size_t offset) const;

    };

    /** Is this tensor explicitly filled, or do we generate on the fly? */
    enum class TensorStorageType {
        /** Generate data on the fly. */
        Virtual,
        /** Generate data in advance, then read. */
        Explicit,
        /** Automatically choose between Virtual and Explicit based on total element count. */
        Automatic
    };

    /**
     * Tensor, that might be virtual or explicit.
     * @tparam elem_t The element type.
     * @tparam threshold The maximum number of elements to store in explicit mode (unless overloaded).
     */
    template<class elem_t, size_t threshold>
    class AutoStorageTensor : public Tensor {
    protected:
        constexpr static const size_t automated_storage_threshold = threshold;

        using Element = elem_t;

    public:
        const TensorStorageType StorageType;

    protected:
        std::vector<Element> data;

    public:
        explicit AutoStorageTensor(std::vector<size_t>&& dimensions,
                                   TensorStorageType storage = TensorStorageType::Automatic)
         : Tensor{std::move(dimensions)}, StorageType(get_storage_type(storage, ElementCount)) { }

         virtual ~AutoStorageTensor() noexcept = default;


        const std::vector<Element>& Data() const {
            if (this->StorageType != TensorStorageType::Explicit) {
                throw errors::bad_tensor::no_data_stored(this->get_name());
            }
            return this->data;
        }

        Element operator()(const Tensor::IndexView indices) const {
            this->validate_index(indices);
            if (this->StorageType == TensorStorageType::Explicit) {
                size_t offset = this->index_to_offset_no_checks(indices);
                return this->data[offset];
            } else {
                return this->make_element_no_checks(indices);
            }
        }

    public:
        static constexpr TensorStorageType get_storage_type(const TensorStorageType hint, const size_t num_elems) {
            if (hint != TensorStorageType::Automatic) {
                return hint;
            }
            if (num_elems > AutoStorageTensor::automated_storage_threshold) {
                return TensorStorageType::Virtual;
            } else {
                return TensorStorageType::Explicit;
            }
        }


    protected:
        [[nodiscard]] virtual Element make_element_no_checks(Tensor::IndexView index) const = 0;

        [[nodiscard]] virtual std::string get_name() const {
            return "Tensor";
        }

    };


}