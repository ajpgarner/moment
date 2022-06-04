/*
 * multi_operator_iterator.h
 *
 * (c) 2022 Austrian Academy of Sciences.
 */
#pragma once
#include "operator_collection.h"
#include "operator_sequence.h"

#include <vector>
#include <iterator>

namespace NPATK::detail {
    class MultiOperatorIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = OperatorSequence;

    private:
        const OperatorCollection *context = nullptr;
        size_t length = 0;
        std::vector <OperatorCollection::AllOperatorConstIterator> iters{};

    public:
        /** 'Begin' iterator */
        constexpr MultiOperatorIterator(const OperatorCollection &the_context, size_t max_length)
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

        constexpr MultiOperatorIterator operator++(int) &{
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

        OperatorSequence operator*() const noexcept {
            std::vector <Operator> raw{};
            raw.reserve(length);
            // Reverse order so final operator in chain changes the most frequently:
            for (size_t i = length; i > 0; --i) {
                raw.emplace_back(*(this->iters[i-1]));
            }
            return OperatorSequence{std::move(raw), this->context};
        }


    private:

        /** 'End' iterator, private c'tor */
        constexpr MultiOperatorIterator(const OperatorCollection &the_context, size_t max_length, bool)
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
        constexpr static MultiOperatorIterator end_of(const OperatorCollection &context, size_t max_length) {
            return MultiOperatorIterator{context, max_length, true};
        }

    };

    class MultiOperatorRange {
    private:
         const OperatorCollection& context;
         size_t length{};
    public:
        MultiOperatorRange(const OperatorCollection& context, size_t length)
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