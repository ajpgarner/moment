/**
 * queue.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * See also: chapter 3 of A. Williams, "C++ Concurrency in action".
 */

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

#include <cassert>

namespace Moment::Multithreading {

    /**
     * Thread-safe single-linked list queue with head/tail mutex.
     * @tparam elem_t Stored element
     */
    template<typename elem_t>
    class Queue {

    public:
        struct Node {
        public:
            struct dummy_node_tag{};

        private:
            std::optional<elem_t> datum;
            std::unique_ptr<Node> next;

        public:
            /** Dummy node constructor. */
            explicit Node(dummy_node_tag&& /**/) noexcept : next{nullptr} { }

            /** Move data constructor */
            explicit Node(elem_t input_datum) noexcept : datum{std::move(input_datum)} { }

            /** Emplacement constructor. */
            template<typename... Args>
            explicit Node(Args&&... args)
                : datum{std::forward<Args>(args)...} { }


            /** Nodes should not be copied. */
            Node(const Node& rhs) = delete;



        public:
            elem_t& operator*() noexcept {
                return datum.value();
            }
            const elem_t& operator*() const noexcept {
                return datum.value();
            }
            elem_t* operator->() noexcept {
                return &(datum.value());
            }
            const elem_t* operator->() const noexcept {
                return &(datum.value());
            }
            friend class Queue;
        };

    private:
        std::unique_ptr<Node> head;

        Node * tail;

        mutable std::mutex head_mutex, tail_mutex;
        mutable std::condition_variable data_cv;

        std::atomic_flag abort_flag;


    public:
        // Queue always initializes with head/tail as a dummy node.
        Queue() : head{std::make_unique<Node>(typename Node::dummy_node_tag{})}, tail{head.get()} {
            this->abort_flag.clear();
        }


        template<typename... Args>
        void emplace_back(Args&&...args) {
            auto next_node = std::make_unique<Node>(typename Node::dummy_node_tag{});

            {
                std::lock_guard tail_lock{this->tail_mutex};
                assert(this->consistent_tail());

                this->tail->datum.emplace(std::forward<Args>(args)...);
                this->tail->next = std::move(next_node);
                this->tail = this->tail->next.get();
            }
            this->data_cv.notify_one();
            // ~tail_lock
        }

        void push_back(elem_t object) {
            auto next_node = std::make_unique<Node>(typename Node::dummy_node_tag{});

            {
                std::lock_guard tail_lock{this->tail_mutex};
                assert(this->consistent_tail());

                this->tail->datum.emplace(std::move(object));
                this->tail->next = std::move(next_node);
                this->tail = this->tail->next.get();
            }
            this->data_cv.notify_one();
            // ~tail_lock
        }

        void abort() noexcept {
            this->abort_flag.test_and_set(std::memory_order_release);
            this->data_cv.notify_all();
        }

        [[nodiscard]] inline bool aborting() const noexcept {
            return this->abort_flag.test(std::memory_order_acquire);
        }


        std::unique_ptr<Node> try_pop_front() noexcept {
            std::lock_guard head_lock{this->head_mutex};

            // Get nothing if abortion triggered
            if (this->aborting()) {
                return nullptr;
            }

            // Empty if just one (dummy) node?
            if (this->head.get() == this->get_tail()) {
                return nullptr;
            }

            // Otherwise, get head and return
            return pop_head();
            // ~head_lock
        }

        std::unique_ptr<Node> wait_pop_front() noexcept {
            auto [head_lock, is_aborting] = this->wait_for_data();
            if (is_aborting) {
                return nullptr;
                // ~head_lock
            }

            // Get head and return
            return pop_head();

            // ~head_lock
        }

    private:
        /**
         * Locks the head of queue, and waits until a consistent data entry is ready.
         */
        [[nodiscard]] std::tuple<std::unique_lock<std::mutex>, bool> wait_for_data() const noexcept {

            std::tuple<std::unique_lock<std::mutex>, bool> output{this->head_mutex, false};

            // Wait on CV
            this->data_cv.wait(std::get<0>(output), [this]() -> bool {
                return (this->head.get() != this->get_tail()) || this->aborting();
            });

            std::get<1>(output) = this->abort_flag.test(std::memory_order_relaxed);

            return output;
        }

        /**
         * Pops the head off the list.
         * A lock on head_mutex should be held by caller.
         */
        [[nodiscard]] inline std::unique_ptr<Node> pop_head() noexcept  {
            std::unique_ptr<Node> output{std::move(this->head->next)};
            output.swap(this->head);
            return output;
        }

        [[nodiscard]] inline const Node * get_tail() const noexcept {
            std::lock_guard tail_lock{this->tail_mutex};
            return this->tail;
        }

        /**
         * Check that the tail is in a consistent state.
         * If a lock on tail_mutex is held, will always return true.
         * If a lock is not held, may spuriously return false.
         */
        [[nodiscard]] inline bool consistent_tail() const noexcept {
            return this->tail && !this->tail->datum.has_value() && !this->tail->next;
        }
    };

}