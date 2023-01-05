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

    /**
     * Vector, with optimized stack storage for short lengths
     * @tparam value_t The value type stored in the vector. Must be default-constructable, and trivially copiable.
     * @tparam SmallN The number of values to store on the stack, before heap storage is required.
     */
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
        /**
         * Default (empty, on stack) constructor.
         */
        constexpr SmallVector() : data_start{stack_data.data()} { }

        /**
         * Copy constructor.
         * @param rhs Source.
         */
        SmallVector(const SmallVector& rhs) : _size{rhs._size}, _capacity{rhs._capacity} {
            if (rhs.heap_data) {
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                std::span<const value_t> other_view{rhs.heap_data.get(), this->_size};
                std::span<value_t> this_view{this->heap_data.get(), this->_capacity};
                std::copy(other_view.begin(), other_view.end(), this_view.begin());
                this->data_start = this->heap_data.get();
            } else {
                std::copy(rhs.stack_data.cbegin(), rhs.stack_data.cbegin() + rhs._size, this->stack_data.begin());
                this->data_start = this->stack_data.data();
            }
        }

        /**
         * Move constructor. Note: if vector is small enough to be on stack, it must be copied.
         * @param rhs Source object. Must not be used after being moved from!
         */
        constexpr SmallVector(SmallVector&& rhs) noexcept
            : _size{rhs._size}, _capacity{rhs._capacity}, heap_data{std::move(rhs.heap_data)} {
            if (this->heap_data) {
                this->data_start = this->heap_data.get();

                // Reset RHS to empty stack vector
                rhs.data_start = rhs.stack_data.data();
                rhs._size = 0;
                rhs._capacity = SmallN;
            } else {
                // stack data must be manually moved
                std::move(rhs.stack_data.begin(), rhs.stack_data.begin() + rhs._size,
                          this->stack_data.begin());
                this->data_start = this->stack_data.data();
                // RHS is left untouched ~
            }
        }

        /**
         * Construct small vector, copying data from iterator
         *
         * @tparam input_iterator The type of iterator
         * @param iter Start of data to copy into container
         * @param iter_end Must be reachable from iter.
         */
        template<class input_iterator>
        SmallVector(input_iterator iter, input_iterator iter_end)
            : _size{static_cast<size_t>(std::distance(iter, iter_end))} {
            if (_size <= SmallN) {
                std::copy(iter, iter_end, this->stack_data.data());
                this->data_start = this->stack_data.data();
            } else {
                this->_capacity = suggest_capacity(this->_size);
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                this->data_start = this->heap_data.get();

                std::span<value_t> heap_view{this->heap_data.get(), this->_capacity};
                std::copy(iter, iter_end, heap_view.begin());
            }
        }

        /**
         * Construct vector from initializer list
         */
        SmallVector(std::initializer_list<value_t> initial_data)
                : _size{initial_data.size()} {
            if (initial_data.size() <= SmallN) {
                std::move(initial_data.begin(), initial_data.end(), this->stack_data.begin());
                this->data_start = this->stack_data.data();
            } else {
                this->_capacity = suggest_capacity(this->_size);
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                this->data_start = this->heap_data.get();

                std::span<value_t> heap_view{this->heap_data.get(), this->_capacity};
                std::move(initial_data.begin(), initial_data.end(), heap_view.begin());
            }
        }

        /**
         * Destructor.
         */
        ~SmallVector() = default;

        /**
         * Access data by pointer.
         */
        [[nodiscard]] constexpr value_t* get() noexcept {
            return this->data_start;
        }

        /**
         * Access data by pointer.
         */
        [[nodiscard]] constexpr const value_t* get() const noexcept {
            return this->data_start;
        }

        /**
         * Access data by index.
         */
        [[nodiscard]] constexpr value_t& operator[](size_t index) {
            return this->data_start[index];
        }

        /**
         * Access data by index.
         */
        [[nodiscard]] constexpr const value_t& operator[](size_t index) const {
            return this->data_start[index];
        }

        /**
         * True if container has no elements.
         */
        [[nodiscard]] constexpr bool empty() const noexcept {
            return (this->_size == 0);
        }

        /**
         * Removes content of vector.
         * This does not necessarily shrink capacity.
         */
        constexpr void clear() noexcept {
            this->_size = 0;
        }

        /**
         * Number of elements in the container.
         */
        [[nodiscard]] constexpr size_t size() const noexcept {
            return this->_size;
        }

        /**
         * Size of reserved memory block.
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
         * Add value at end of vector.
         */
        void push_back(value_t elem) {
            // Check if we need to expand capacity
            if (this->_size >= this->_capacity) {
                this->reallocate(this->_size+1);
            }

            // Move element to new location
            value_t * location = this->data_start + this->_size;
            new(location) value_t(std::move(elem));
            this->_size += 1;
        }

        /**
         * Construct object, and push to back of vector.
         * Note: pure emplace_back doesn't really exist, as value_t must be value type.
         */
        template<class... Args>
        void emplace_back(Args&&... args) {
            // Check if we need to expand capacity
            if (this->_size >= this->_capacity) {
                this->reallocate(this->_size+1);
            }

            // Construct element in situ:
            value_t * location = this->data_start + this->_size;
            new(location) value_t(std::forward<Args>(args)...);
            this->_size += 1;
        }

        /**
         * Iterator to beginning of vector
         */
        iterator begin() noexcept {
            return this->data_start;
        }

        /**
         * Constant iterator to beginning of vector
         */
        const_iterator begin() const noexcept {
            return this->data_start;
        }

        /**
         * Constant iterator to beginning of vector
         */
        const_iterator cbegin() const noexcept {
            return this->data_start;
        }

        /**
         * Iterator to point past end of vector
         */
        iterator end() noexcept {
            return this->data_start + this->_size;
        }

        /**
         * Constant iterator to point past end of vector
         */
        const_iterator end() const noexcept {
            return this->data_start + this->_size;
        }

        /**
         * Constant iterator to point past end of vector
         */
        const_iterator cend() const noexcept {
            return this->data_start + this->_size;
        }


        /**
         * Ensure the vector has sufficient capacity to accommodate requested_storage number of elements
         */
        void reserve(const size_t requested_storage) {
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
            std::move(this->stack_data.begin(), this->stack_data.end(), new_heap_view.begin());
            this->_capacity = new_capacity;
            this->data_start = this->heap_data.get();
        }

        void reallocate_from_heap_to_heap(const size_t required_size) {
            auto new_capacity = suggest_capacity(required_size);
            auto new_heap = std::make_unique<value_t[]>(new_capacity);
            std::span<const value_t> old_heap_view{this->heap_data.get(), this->_capacity};
            std::span<value_t> new_heap_view{new_heap.get(), new_capacity};
            std::move(old_heap_view.begin(), old_heap_view.end(), new_heap_view.begin());
            this->heap_data.swap(new_heap);
            this->_capacity = new_capacity;
            this->data_start = this->heap_data.get();
        }
    };

}