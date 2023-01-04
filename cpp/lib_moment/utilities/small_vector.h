/**
 * small_vector.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include <array>
#include <bit>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <span>

namespace Moment {

    template<typename value_t, size_t SmallN>
    class SmallVector {

    public:
        using iterator = value_t *;
        using const_iterator = const value_t *;

    private:
        std::unique_ptr<value_t[]> heap_data;
        std::array<value_t, SmallN> stack_data;

        size_t _size = 0;
        size_t _capacity = SmallN;

        value_t * data_start;

    public:
        constexpr SmallVector() : data_start{stack_data.data()} { }

        SmallVector(std::initializer_list<value_t> initial_data)
            : _size{initial_data.size()} {
            if (initial_data.size() <= SmallN) {
                std::copy(initial_data.begin(), initial_data.end(), this->stack_data.begin());
                this->data_start = this->stack_data.data();
            } else {
                this->_capacity = suggest_capacity(this->_size);
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                this->data_start = this->heap_data.get();

                std::span<value_t> heap_view{this->heap_data.get(), this->_capacity};
                std::copy(initial_data.begin(), initial_data.end(), heap_view.begin());
            }
        }

        ~SmallVector() = default;

        std::span<value_t> span() {
            return std::span(this->data_start, _size);
        }

        /**
         * Access data by index.
         */
        [[nodiscard]] value_t& operator[](size_t index) {
            return this->data_start[index];
        }

        /**
         * Access data by index.
         */
        [[nodiscard]] const value_t& operator[](size_t index) const {
            return this->data_start[index];
        }

        /**
         * True if container has no elements.
         */
        [[nodiscard]] constexpr bool empty() const noexcept {
            return (this->_size == 0);
        }

        /**
         * Number of elements in the container.
         */
        [[nodiscard]] constexpr size_t size() const noexcept {
            return this->_size;
        }

        /**
         * Size of reserved memory block
         */
        [[nodiscard]] constexpr size_t capacity() const noexcept {
            return this->_capacity;
        }

        /**
         * True if data is stored on the heap; false if on stack.
         */
        [[nodiscard]] constexpr bool on_heap() const noexcept {
            return static_cast<bool>(this->heap_data);
        }

        /**
         * Add value to end of vector
         */
        void push_back(value_t elem) {
            // We are at capacity, so expand...
            if (this->_size >= this->_capacity) {
                this->reallocate(this->_size+1);
            }

            // Copy element
            this->data_start[this->_size] = std::move(elem);
            this->_size += 1;
        }

        iterator begin() {
            return this->data_start;
        }

        const_iterator begin() const {
            return this->data_start;
        }

        const_iterator cbegin() const {
            return this->data_start;
        }

        iterator end() {
            return this->data_start + this->_size;
        }

        const_iterator end() const {
            return this->data_start + this->_size;
        }

        const_iterator cend() const {
            return this->data_start + this->_size;
        }

        inline void reserve(const size_t requested_storage) {
            // Ignore, if capacity is already there
            if (requested_storage > this->_capacity) {
                this->reallocate(requested_storage);
            }
        }

    private:
        [[nodiscard]] constexpr static size_t suggest_capacity(const size_t required_size) noexcept {
            return std::bit_ceil(required_size);
        }

        inline void reallocate(const size_t required_size) {
            if (this->heap_data) {
                this->reallocate_from_heap_to_heap(required_size);
            } else {
                this->reallocate_from_stack_to_heap(required_size);
            }
        }

        void reallocate_from_stack_to_heap(const size_t required_size) {
            auto new_capacity = suggest_capacity(required_size);
            this->heap_data = std::make_unique<value_t[]>(new_capacity);
            std::span<value_t> new_heap_view{this->heap_data.get(), new_capacity};
            std::copy(this->stack_data.cbegin(), this->stack_data.cend(), new_heap_view.begin());
            this->_capacity = new_capacity;
            this->data_start = this->heap_data.get();
        }

        void reallocate_from_heap_to_heap(const size_t required_size) {
            auto new_capacity = suggest_capacity(required_size);
            auto new_heap = std::make_unique<value_t[]>(new_capacity);
            std::span<const value_t> old_heap_view{this->heap_data.get(), this->_capacity};
            std::span<value_t> new_heap_view{new_heap.get(), new_capacity};
            std::copy(old_heap_view.begin(), old_heap_view.end(), new_heap_view.begin());
            this->heap_data.swap(new_heap);
            this->_capacity = new_capacity;
            this->data_start = this->heap_data.get();
        }



    };

}