/**
 * small_vector.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
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
        using value_type = value_t;

    public:
        using iterator = value_t *;
        using const_iterator = const value_t *;

    private:
        std::unique_ptr<value_t[]> heap_data{nullptr};
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
        constexpr SmallVector(const SmallVector& rhs) : _size{rhs._size}, _capacity{rhs._capacity} {
            if (rhs.heap_data) {
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                std::copy(rhs.heap_data.get(), rhs.heap_data.get() + rhs._size, this->heap_data.get());
                this->data_start = this->heap_data.get();
            } else {
                std::copy(rhs.stack_data.cbegin(), rhs.stack_data.cbegin() + rhs._size, this->stack_data.begin());
                this->data_start = this->stack_data.data();
            }
        }

        /**
         * Copy assignment.
         * @param rhs Source.
         */
        constexpr SmallVector& operator=(const SmallVector& rhs) {
            this->_size = rhs._size;
            this->_capacity = rhs._capacity;
            if (rhs.heap_data) {
                this->heap_data = std::make_unique<value_t[]>(this->_capacity); // release existing ptr.
                std::copy(rhs.heap_data.get(), rhs.heap_data.get() + rhs._size, this->heap_data.get());
                this->data_start = this->heap_data.get();
            } else {
                std::copy(rhs.stack_data.cbegin(), rhs.stack_data.cbegin() + rhs._size, this->stack_data.begin());
                this->data_start = this->stack_data.data();
            }
            return *this;
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
         * Move assignment
         */
         constexpr SmallVector& operator=(SmallVector&& rhs) noexcept {
             if (rhs.heap_data) {
                 // Move and overwrite LHS
                 this->heap_data = std::move(rhs.heap_data); // release existing ptr.
                 this->data_start = this->heap_data.get();
                 this->_size = rhs._size;
                 this->_capacity = rhs._capacity;

                 // Reset RHS
                 rhs.data_start = rhs.stack_data.data();
                 rhs._size = 0;
                 rhs._capacity = SmallN;
             } else {
                 // Stack data must be copied
                 std::move(rhs.stack_data.begin(), rhs.stack_data.begin() + rhs._size,
                           this->stack_data.begin());
                 this->data_start = this->stack_data.data();
                 this->_size = rhs._size;
                 this->_capacity = SmallN;
                 // RHS is left untouched ~
             }
            return *this;
         }

        /**
         * Construct small vector, copying data from iterator
         *
         * @tparam input_iterator The type of iterator
         * @param iter Start of data to copy into container
         * @param iter_end Must be reachable from iter.
         */
        template<class input_iterator>
        constexpr SmallVector(input_iterator iter, input_iterator iter_end)
            : _size{static_cast<size_t>(std::distance(iter, iter_end))} {
            if (_size <= SmallN) {
                std::copy(iter, iter_end, this->stack_data.data());
                this->data_start = this->stack_data.data();
            } else {
                this->_capacity = suggest_capacity(this->_size);
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                this->data_start = this->heap_data.get();
                std::copy(iter, iter_end, this->heap_data.get());
            }
        }

        /**
         * Construct vector from initializer list
         */
        constexpr SmallVector(std::initializer_list<value_t> initial_data) : _size{initial_data.size()} {
            if (initial_data.size() <= SmallN) {
                std::move(initial_data.begin(), initial_data.end(), this->stack_data.begin());
                this->data_start = this->stack_data.data();
            } else {
                this->_capacity = suggest_capacity(this->_size);
                this->heap_data = std::make_unique<value_t[]>(this->_capacity);
                this->data_start = this->heap_data.get();

                std::move(initial_data.begin(), initial_data.end(), this->heap_data.get());
            }
        }

        /**
         * Destructor.
         */
        constexpr ~SmallVector() = default;

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
         * Insert into container
         */
        template<class input_iter_t>
        void insert(iterator where, input_iter_t source_iter, input_iter_t source_iter_end) {
            const auto amount_before_insert = static_cast<size_t>(std::distance(this->data_start, where));
            const auto amount_to_insert = static_cast<size_t>(std::distance(source_iter, source_iter_end));
            const bool insert_at_back = (amount_before_insert == this->_size);

            // Can we do move without reallocation?
            if (this->_size + amount_to_insert <= this->_capacity) {
                // First, move tail of elements to make space for new insertion
                if (!insert_at_back) {
                    std::move_backward(where, this->data_start + this->_size,
                                       this->data_start + this->_size + amount_to_insert);
                }
                // Now, copy (cf. move with appropriate input_iter_t) new elements into container
                std::copy(source_iter, source_iter_end, where);
                this->_size += amount_to_insert;
            } else {
                const size_t new_capacity = suggest_capacity(this->_size+amount_to_insert);
                auto new_heap = std::make_unique<value_t[]>(new_capacity);

                // Move previous bits before allocation; copy new chunk in middle
                std::move(this->data_start, where, new_heap.get());
                std::copy(source_iter, source_iter_end, new_heap.get() + amount_before_insert);
                if (!insert_at_back) {
                    std::move(where, this->data_start + this->_size,
                              new_heap.get() + amount_before_insert + amount_to_insert);
                }

                // Set copy as new data block
                this->heap_data = std::move(new_heap);
                this->_size += amount_to_insert;
                this->_capacity = new_capacity;
                this->data_start = this->heap_data.get();
            }
        }


        /**
         * Cut range from container
         */
        iterator erase(iterator where_from, iterator where_to) {
            const auto elements_trimmed = static_cast<size_t>(std::distance(where_from, where_to));
            std::move(where_to, this->data_start + this->_size, where_from);
            this->_size -= elements_trimmed;
            return where_from;
        }

        /**
         * Iterator to beginning of vector
         */
        constexpr iterator begin() noexcept {
            return this->data_start;
        }

        /**
         * Constant iterator to beginning of vector
         */
        constexpr const_iterator begin() const noexcept {
            return this->data_start;
        }

        /**
         * Constant iterator to beginning of vector
         */
        constexpr const_iterator cbegin() const noexcept {
            return this->data_start;
        }

        /**
         * Iterator to point past end of vector
         */
        constexpr iterator end() noexcept {
            return this->data_start + this->_size;
        }

        /**
         * Constant iterator to point past end of vector
         */
        constexpr const_iterator end() const noexcept {
            return this->data_start + this->_size;
        }

        /**
         * Constant iterator to point past end of vector
         */
        constexpr const_iterator cend() const noexcept {
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

        /**
         * Swap vector contents: take contents of RHS into this vector, and put this vector's contents into RHS.
         */
        void swap(SmallVector& rhs) noexcept {
            if (this->heap_data) {
                if (rhs.heap_data) {
                    std::swap(this->heap_data, rhs.heap_data);
                    std::swap(this->data_start, rhs.data_start);
                } else {
                    // copy/move RHS stack to our stack, give RHS our heap
                    rhs.heap_data = std::move(this->heap_data);
                    rhs.data_start = rhs.heap_data.get();
                    std::move(rhs.stack_data.begin(), rhs.stack_data.begin() + rhs._size, this->stack_data.begin());
                    this->data_start = this->stack_data.data();
                }
            } else {
                if (rhs.heap_data) {
                    // copy/move our stack to RHS stack, take RHS's heap
                    this->heap_data = std::move(rhs.heap_data);
                    this->data_start = this->heap_data.get();
                    std::move(this->stack_data.begin(), this->stack_data.begin() + this->_size, rhs.stack_data.begin());
                    rhs.data_start = rhs.stack_data.data();
                } else {
                    // swap stack data
                    size_t common_size = std::min(this->_size, rhs._size);
                    for (size_t i = 0; i < common_size; ++i) {
                        std::swap(this->stack_data[i], rhs.stack_data[i]);
                    }
                    if (this->_size > common_size) {
                        std::move(this->stack_data.begin() + common_size, this->stack_data.begin() + this->_size,
                                  rhs.stack_data.begin() + common_size);
                    } else if (rhs._size > common_size) {
                        std::move(rhs.stack_data.begin() + common_size, rhs.stack_data.begin() + rhs._size,
                                  this->stack_data.begin() + common_size);
                    }
                }
            }

            // Swap sizes and capacities
            std::swap(this->_size, rhs._size);
            std::swap(this->_capacity, rhs._capacity);
        }

        /**
         * Exchange left and right hand arguments.
         */
        friend void swap(SmallVector& lhs, SmallVector& rhs) noexcept {
            lhs.swap(rhs);
        }



        /**
         * Degrade to span
         */
        constexpr operator ::std::span<value_t>() noexcept {
            return std::span<value_t>{this->data_start, this->_size};
        }

        /**
         * Degrade to span over constant values
         */
        constexpr operator ::std::span<const value_t>() const noexcept {
            return std::span<const value_t>{this->data_start, this->_size};
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