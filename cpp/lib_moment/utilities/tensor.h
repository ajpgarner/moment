/**
 * tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multi_dimensional_offset_index_iterator.h"

#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <variant>
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
         * Destructor.
         */
        virtual ~Tensor() noexcept = default;

        /**
         * Check that an index has the right number of elements, and is in range.
         * @throws bad_tensor_index if indices are invalid
         */
        void validate_index(IndexView indices) const;

        /**
         * Check that an index has the right number of elements, and is either in range or at the end of the range.
         * @throws bad_tensor_index if indices are invalid
         */
        void validate_index_inclusive(IndexView indices) const;

        /**
         * Check that a pair of indices has the right number of elements, are in bounds, and refer to a positive range.

         */
        void validate_range(IndexView min, IndexView max) const;

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

        /**
         * Name of tensor object.
         */
        [[nodiscard]] virtual std::string get_name(bool capital) const {
            if (capital) {
                return "Tensor";
            } else {
                return "tensor";
            }
        }

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
    public:
        using Element = elem_t;

    public:

        /**
         * Holds either a pointer to data in the tensor, or a copy of the data itself.
         */
        class ElementView {
        private:
            struct flag_no_checks{

            };

            std::variant<const Element *, Element> view;

        public:
            /** Get view into tensor, constructing virtual object if necessary */
            ElementView(const AutoStorageTensor& tensor, const Tensor::IndexView index) {
                tensor.validate_index(index);
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    const size_t offset = tensor.index_to_offset_no_checks(index);
                    this->view.template emplace<0>(&tensor.data[offset]);
                } else {
                    this->view.template emplace<1>(tensor.make_element_no_checks(index));
                }
            }

            /** Get view into tensor, constructing virtual object if necessary */
            ElementView(const AutoStorageTensor& tensor, size_t offset) {
                tensor.validate_offset(offset);
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    this->view.template emplace<0>(&tensor.data[offset]);
                } else {
                    const auto index = tensor.offset_to_index_no_checks(offset);
                    this->view.template emplace<1>(tensor.make_element_no_checks(index));
                }
            }

        private:
            /** Get view into tensor, constructing virtual object if necessary */
            ElementView(const AutoStorageTensor& tensor, const Tensor::IndexView index, const flag_no_checks& /**/) {
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    const size_t offset = tensor.index_to_offset_no_checks(index);
                    this->view.template emplace<0>(&tensor.data[offset]);
                } else {
                    this->view.template emplace<1>(tensor.make_element_no_checks(index));
                }
            }

            ElementView(const AutoStorageTensor& tensor, size_t offset, const flag_no_checks& /**/) {
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    this->view.template emplace<0>(&tensor.data[offset]);
                } else {
                    const auto index = tensor.offset_to_index_no_checks(offset);
                    this->view.template emplace<1>(tensor.make_element_no_checks(index));
                }
            }

        public:
            operator const Element&() const { // NOLINT(google-explicit-constructor)
                if (this->view.index() == 0) {
                    return *std::get<0>(this->view);
                } else {
                    return std::get<1>(this->view);
                }
            }

            inline const Element* operator->() const {
                if (this->view.index() == 0) {
                    return std::get<0>(this->view);
                } else {
                    return &std::get<1>(this->view);
                }
            }

            friend class AutoStorageTensor<elem_t, threshold>;
        };

        /**
         * Splice iterator.
         * Iterators must not be shared between threads, due to mutable virtual entry.
         */
        class Iterator {
        protected:
            /** Pointer to tensor. */
            const AutoStorageTensor *tensorPtr;

            /** Evaluated current entry (only in virtual mode). */
            mutable std::optional<Element> virtual_entry;

            /** Index, in tensor indices. */
            MultiDimensionalOffsetIndexIterator<true, Tensor::Index> mdoii;

            /** Global offset within the tensor. */
            size_t current_offset = 0;

        public:
            /**
             * Construct iterator over supplied index range.
             */
            Iterator(const AutoStorageTensor<elem_t, threshold>& tensor, Tensor::Index&& first, Tensor::Index&& last)
                :  tensorPtr{&tensor}, mdoii{std::move(first), std::move(last)} {
                if (this->mdoii) {
                    this->current_offset = this->tensorPtr->index_to_offset_no_checks(*this->mdoii);
                }
            }

            /**
             * 'End' iterator constructor.
             */
            explicit Iterator(const AutoStorageTensor& tensor) : tensorPtr{&tensor}, mdoii{}, current_offset{0} { }

            /**
             * Increment iterator.
             */
            Iterator& operator++() noexcept {
                this->virtual_entry.reset();
                ++this->mdoii;
                if (this->mdoii) {
                    this->current_offset = this->tensorPtr->index_to_offset_no_checks(*this->mdoii);
                } else {
                    this->current_offset = 0;
                }
                return *this;
            }

            /**
             * True, if iterator is not done.
             */
            [[nodiscard]] inline explicit operator bool() const noexcept {
                return static_cast<bool>(this->mdoii);
            }

            /**
             * True, if iterator is done.
             */
            [[nodiscard]] inline bool operator!() const noexcept {
                return !this->mdoii;
            }

            /**
             * Gets current CG index.
             */
            [[nodiscard]] inline Tensor::IndexView index() const noexcept {
                return *this->mdoii;
            }

            /**
            * Gets current element (cache if necessary).
            */
            [[nodiscard]] const Element& operator*() const {
                if (this->tensorPtr->StorageType == TensorStorageType::Explicit) {
                    return this->tensorPtr->data[this->current_offset];
                }
                if (!this->virtual_entry.has_value()) {
                    this->virtual_entry = this->tensorPtr->make_element_no_checks(*this->mdoii);
                }
                return this->virtual_entry.value();
            }

            /**
            * Gets current element (cache if necessary).
            */
            [[nodiscard]] const Element* operator->() const {
                if (this->tensorPtr->StorageType == TensorStorageType::Explicit) {
                    return &this->tensorPtr->data[this->current_offset];
                }
                if (!this->virtual_entry.has_value()) {
                    this->virtual_entry = this->tensorPtr->make_element_no_checks(*this->mdoii);
                }
                return &this->virtual_entry.value();
            }

            /**
             * Equality test.
             */
            [[nodiscard]] inline bool operator==(const Iterator& other) const noexcept {
                return this->mdoii == other.mdoii;
            }

            /**
             * Inequality test.
             */
            [[nodiscard]] inline bool operator!=(const Iterator& other) const noexcept {
                return this->mdoii != other.mdoii;
            }

            /**
             * Gets offset within splice represented by this iterator.
             */
            [[nodiscard]] inline size_t block_offset() const noexcept {
                return mdoii.global();
            }

            /**
             * Gets offset within entire tensor.
             */
            [[nodiscard]] inline size_t offset() const noexcept {
                return this->current_offset;
            }
        };


        /**
         * Iterator over entire tensor.
         */
        class FullIterator {
        public:
            struct end_tag_t{};
        private:
            const AutoStorageTensor* tensorPtr;
            std::variant<typename std::vector<Element>::const_iterator, Iterator> implIter;
            std::optional<typename std::vector<Element>::const_iterator> directIterEnd;

        public:
            FullIterator(const AutoStorageTensor& tensor) : tensorPtr(&tensor) {
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    this->implIter.template emplace<0>(tensor.data.cbegin());
                    this->directIterEnd = tensor.data.cend();
                } else {
                    this->implIter.template emplace<1>(tensor,
                            Tensor::Index(tensor.DimensionCount, 0),
                            Tensor::Index(tensor.Dimensions));
                    this->directIterEnd = std::nullopt;
                }
            }

            FullIterator(const AutoStorageTensor& tensor, end_tag_t) : tensorPtr(&tensor) {
                if (tensor.StorageType == TensorStorageType::Explicit) {
                    this->implIter.template emplace<0>(tensor.data.cend());
                    this->directIterEnd = tensor.data.cend();
                } else {
                    this->implIter.template emplace<1>(tensor);
                    this->directIterEnd = std::nullopt;
                }
            }

            [[nodiscard]] inline bool explicit_mode() const noexcept {
                return implIter.index() == 0;
            }


            Index index() const {
                if (implIter.index() == 0) {
                    const size_t offset = std::distance(tensorPtr->data.cbegin(), std::get<0>(implIter));
                    return tensorPtr->offset_to_index_no_checks(offset);
                } else {
                    const auto index_view = std::get<1>(implIter).index();
                    return Index(index_view.begin(), index_view.end());
                }
            }

            size_t offset() const {
                if (implIter.index() == 0) {
                    return std::distance(tensorPtr->data.cbegin(), std::get<0>(implIter));
                } else {
                    const auto index_view = std::get<1>(implIter).index();
                    return tensorPtr->index_to_offset_no_checks(index_view);
                }
            }

            FullIterator& operator++() noexcept {
                if (implIter.index() == 0) {
                    ++std::get<0>(implIter);
                } else {
                    ++std::get<1>(implIter);
                }
                return *this;
            }

            [[nodiscard]] const Element& operator*() const {
                if (implIter.index() == 0) {
                    return *std::get<0>(implIter);
                } else {
                    return *std::get<1>(implIter);
                }
            }

            [[nodiscard]] const Element* operator->() const  {
                if (implIter.index() == 0) {
                    return std::get<0>(implIter).operator->();
                } else {
                    return std::get<1>(implIter).operator->();
                }
            }

            [[nodiscard]] bool operator==(const FullIterator& rhs) const noexcept {
                assert(rhs.implIter.index() == this->implIter.index());
                if (this->implIter.index() == 0) {
                    return std::get<0>(this->implIter) == std::get<0>(rhs.implIter);
                } else {
                    return std::get<1>(this->implIter) == std::get<1>(rhs.implIter);
                }
            }

            [[nodiscard]] inline bool operator!=(const FullIterator& rhs) const noexcept {
                return !this->operator==(rhs);
            }

            [[nodiscard]] operator bool() const {
                if (this->implIter.index() == 0) {
                    assert(this->directIterEnd.has_value());
                    return (std::get<0>(this->implIter) != this->directIterEnd.value());
                } else {
                    return std::get<1>(this->implIter).operator bool();
                }
            }

            [[nodiscard]] bool operator!() const {
                if (this->implIter.index() == 0) {
                    assert(this->directIterEnd.has_value());
                    return (std::get<0>(this->implIter) == this->directIterEnd.value());
                } else {
                    return std::get<1>(this->implIter).operator!();
                }
            }


        };

        template<typename tensor_t = AutoStorageTensor>
        class Range {
        private:
            const tensor_t& tensor;
            const Index first;
            const Index last;

            const Iterator iter_end;

        public:
            Range(const tensor_t& tensor, Index&& first, Index&& last)
                    : tensor{tensor}, first(std::move(first)), last(std::move(last)),
                      iter_end{tensor} { }

            [[nodiscard]] inline Iterator begin() const {
                return Iterator{tensor, Index(this->first), Index(this->last)};
            }

            [[nodiscard]] inline const Iterator& end() const noexcept {
                return this->iter_end;
            }
        };


    public:
        constexpr static const size_t automated_storage_threshold = threshold;

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
                throw errors::bad_tensor::no_data_stored(this->get_name(true));
            }
            return this->data;
        }

        inline ElementView operator()(const Tensor::IndexView indices) const {
            return ElementView{*this, indices};
        }

        /** Iterate over entire tensor */
        [[nodiscard]] FullIterator begin() const {
            return FullIterator{*this};
        }

        /** Iterate over entire tensor */
        [[nodiscard]] FullIterator end() const {
            return FullIterator{*this, typename FullIterator::end_tag_t{}};
        }

        /** Get element by index */
        inline ElementView at(const size_t offset) const {
            return ElementView{*this, offset};
        }

        template<typename range_t = AutoStorageTensor<elem_t, threshold>::Range<AutoStorageTensor<elem_t, threshold>>>
        [[nodiscard]] range_t Splice(Index&& min, Index&& max) {
            this->validate_range(min, max);
            return range_t{*this, std::move(min), std::move(max)};
        }

        template<typename range_t = AutoStorageTensor<elem_t, threshold>::Range<AutoStorageTensor<elem_t, threshold>>>
        [[nodiscard]] range_t Splice(const IndexView minV, const IndexView maxV) {
            this->validate_range(minV, maxV);
            return range_t{*this, Index(minV.begin(), minV.end()), Index(maxV.begin(), maxV.end())};
        }



    protected:
        [[nodiscard]] inline ElementView elem_no_checks(const Tensor::IndexView indices) const {
            return ElementView{*this, indices, typename ElementView::flag_no_checks{}};
        }

        [[nodiscard]] inline ElementView elem_no_checks(const size_t offset) const {
            return ElementView{*this, offset, typename ElementView::flag_no_checks{}};
        }

        [[nodiscard]] virtual Element make_element_no_checks(Tensor::IndexView index) const = 0;


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

    };


}