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

namespace NPATK {
    class MultiOperatorIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:
        const Context *context = nullptr;
        size_t length = 0;
        std::vector <Context::oper_iter_t> iters{};

    public:
        /** 'Begin' iterator */
        constexpr MultiOperatorIterator(const Context &the_context, size_t max_length)
                : context{&the_context}, length{max_length} {
            iters.reserve(length);
            for (size_t i = 0; i < length; ++i) {
                iters.emplace_back(context->begin());
            }
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
                if (this->iters[i] != rhs.iters[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const MultiOperatorIterator &rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] std::vector<Operator> raw() const {
            std::vector<Operator> raw_opers{};
            raw_opers.reserve(length);
            // Reverse order so final operator in chain changes the most frequently:
            for (size_t i = length; i > 0; --i) {
                raw_opers.emplace_back(*(this->iters[i-1]));
            }
            return raw_opers;
        }

        [[nodiscard]] std::vector<oper_name_t> id_str() const {
            std::vector<oper_name_t> output{};
            output.reserve(length);
            // Reverse order so final operator in chain changes the most frequently:
            for (size_t i = length; i > 0; --i) {
                output.emplace_back(this->iters[i-1]->id);
            }
            return output;
        }

        OperatorSequence operator*() const {
            return OperatorSequence{this->raw(), *this->context};
        }



    private:

        /** 'End' iterator, private c'tor */
        constexpr MultiOperatorIterator(const Context &the_context, size_t max_length, bool)
                : context{&the_context}, length{max_length} {
            iters.reserve(length);
            for (size_t i = 0; i < length; ++i) {
                iters.emplace_back(context->end());
            }
        }

        /**
         * Recursively increment iterators.
         */
        constexpr bool inc(size_t depth) noexcept { // NOLINT(misc-no-recursion)
            ++iters[depth];
            if (iters[depth] == context->end()) {
                if ((depth+1) < length) {
                    bool end = inc(depth + 1);
                    if (!end) {
                        iters[depth] = context->begin();
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