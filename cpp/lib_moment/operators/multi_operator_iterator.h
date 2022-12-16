/*
 * multi_operator_iterator.h
 *
 * (c) 2022 Austrian Academy of Sciences.
 */
#pragma once
#include "context.h"
#include "operator_sequence.h"

#include <vector>
#include <iterator>

namespace Moment {
    class MultiOperatorIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:
        const Context *context = nullptr;
        size_t length = 0;
        std::vector<oper_name_t> indices;

    public:
        /** 'Begin' iterator */
        constexpr MultiOperatorIterator(const Context &the_context, const size_t max_length)
                : context{&the_context}, length{max_length}, indices(max_length, 0) {
        }

        MultiOperatorIterator(const MultiOperatorIterator &rhs) = default;


        constexpr MultiOperatorIterator &operator++() noexcept {
            inc(0);
            return *this;
        }

        constexpr MultiOperatorIterator operator++(int) &{ // NOLINT(cert-dcl21-cpp)
            MultiOperatorIterator copy{*this};
            inc(0);
            return copy;
        }

        constexpr bool operator==(const MultiOperatorIterator &rhs) const noexcept {
            assert(this->length == rhs.length);
            assert(this->context == rhs.context);
            for (size_t i = 0; i < length; ++i) {
                if (this->indices[i] != rhs.indices[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const MultiOperatorIterator &rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] std::vector<oper_name_t> raw() const {
            std::vector<oper_name_t> output;
            output.reserve(this->indices.size());
            std::copy(this->indices.crbegin(), this->indices.crend(), std::back_inserter(output));
            return output;
        }

        OperatorSequence operator*() const {
            return OperatorSequence{this->raw(), *this->context};
        }



    private:

        /** 'End' iterator, private c'tor */
        constexpr MultiOperatorIterator(const Context &the_context, size_t max_length, bool)
                : context{&the_context}, length{max_length},
                  indices(max_length, static_cast<oper_name_t>(context->size())) {
        }

        /**
         * Recursively increment iterators.
         */
        constexpr bool inc(size_t depth) noexcept { // NOLINT(misc-no-recursion)
            ++indices[depth];
            if (indices[depth] == context->size()) {
                if ((depth+1) < length) {
                    bool end = inc(depth + 1);
                    if (!end) {
                        indices[depth] = 0;
                    }
                    return end;
                }
                // Reached end of iteration...
                return true;
            }
            // Still more iteration to be done...
            return false;
        }

    public:
        /** 'End' named constructor */
        constexpr static MultiOperatorIterator end_of(const Context &context, size_t max_length) {
            return MultiOperatorIterator{context, max_length, true};
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