/*
 * multi_operator_iterator.h
 *
 * (c) 2022 Austrian Academy of Sciences.
 */
#pragma once
#include "operator_sequence.h"
#include "scenarios/context.h"

#include <vector>
#include <iterator>

namespace Moment {
    class MultiOperatorIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:
        /** Ptr to operator context */
        const Context *context = nullptr;

        /** Length of words generated */
        size_t length = 0;

        /** The minimum op number in sequence. */
        oper_name_t min_op_num = 0;

        /** The maximum op number in sequence. */
        oper_name_t max_op_num = 0;

        /** The operator sequence (without simplification) */
        sequence_storage_t indices;

        /** True if iterator is in end state */
        bool is_done = false;

    public:

        /**
         * Regular 'Begin' iterator
         * @param the_context The operator context.
         * @param max_length The operator word length.
         */
        MultiOperatorIterator(const Context &the_context, const size_t word_length)
                : context{&the_context}, length{word_length}, indices(word_length, 0),
                  min_op_num{0},
                  max_op_num{static_cast<oper_name_t>(context->size())} {
            if ((word_length == 0) || (0 == max_op_num)) {
                this->is_done = true;
            }
        }

        /**
         * Offset 'Begin' iterator
         * @param the_context The operator context.
         * @param max_length The operator word length.
         * @param num_ops The number of different operators
         * @param offset The base operator number.
         */
        MultiOperatorIterator(const Context &the_context, const size_t word_length,
                              const oper_name_t num_ops, const oper_name_t offset)
                : context{&the_context}, length{word_length},
                  indices(word_length, offset),
                  min_op_num{static_cast<oper_name_t>(offset)},
                  max_op_num{static_cast<oper_name_t>(offset + num_ops)} {
            if ((word_length == 0) || (min_op_num == max_op_num)) {
                this->is_done = true;
            }
        }

        MultiOperatorIterator(const MultiOperatorIterator &rhs) = default;

        MultiOperatorIterator &operator++() noexcept {

            auto depth = static_cast<ptrdiff_t>(this->length) - 1;
            while (true) {
                if (depth < 0) {
                    is_done = true;
                    return *this;
                }

                ++this->indices[depth];
                if (this->indices[depth] == this->max_op_num) {
                    this->indices[depth] = this->min_op_num;
                    --depth;
                } else {
                    return *this;
                }
            }
        }

        inline MultiOperatorIterator operator++(int) &{ // NOLINT(cert-dcl21-cpp)
            MultiOperatorIterator copy{*this};
            ++(*this);
            return copy;
        }

        /** Check if done */
        [[nodiscard]] explicit inline operator bool() const noexcept {
            return !this->is_done;
        }

        /** Check if done */
        [[nodiscard]] inline bool operator!() const noexcept {
            return this->is_done;
        }

        /** Comparison between iterators */
        inline bool operator==(const MultiOperatorIterator &rhs) const noexcept {
            assert(this->length == rhs.length);
            assert(this->context == rhs.context);

            if (is_done) {
                return rhs.is_done;
            } else if (rhs.is_done) {
                return false;
            }

            for (size_t i = 0; i < length; ++i) {
                if (this->indices[i] != rhs.indices[i]) {
                    return false;
                }
            }
            return true;
        }

        inline  bool operator!=(const MultiOperatorIterator &rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] const sequence_storage_t& raw() const {
            return this->indices;
        }

        OperatorSequence operator*() const {
            return OperatorSequence{this->raw(), *this->context};
        }



    private:

        /** 'End' iterator, private c'tor */
        constexpr MultiOperatorIterator(const Context &the_context, size_t max_length,
                                        oper_name_t num_ops, oper_name_t offset, bool)
                : context{&the_context}, length{max_length},
                  min_op_num{offset}, max_op_num{static_cast<oper_name_t>(offset+num_ops)},
                  indices(max_length, static_cast<oper_name_t>(0)), is_done{true} {
        }



    public:
        /** 'End' named constructor */
        static MultiOperatorIterator end_of(const Context &context, size_t max_length,
                                            oper_name_t num_ops = -1, oper_name_t offset = 0) {
            return MultiOperatorIterator{context, max_length,
                                         static_cast<oper_name_t>(num_ops >= 0 ? num_ops : context.size()),
                                         offset, true};
        }

    };

    class MultiOperatorRange {
    private:
         const Context& context;
         size_t length{};
    public:
        MultiOperatorRange(const Context& context, size_t length)
            : context{context}, length{length} { }

        [[nodiscard]] auto begin() const {
            return MultiOperatorIterator{context, length};
        }

        [[nodiscard]] auto end() const {
            return MultiOperatorIterator::end_of(context, length);
        }
    };

    static_assert(std::input_iterator< MultiOperatorIterator>);
}