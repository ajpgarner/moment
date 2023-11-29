/**
 * shift_sorter.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "integer_types.h"
#include <initializer_list>
#include <span>

namespace Moment {

    template<typename int_t>
    class ShiftSorter {
    public:

        class ComparePermutationFunctor {
        public:
            const std::span<const int_t> data_view;

        public:
            ComparePermutationFunctor(const std::span<const int_t> data_view) : data_view{data_view} {}

            constexpr inline size_t size() const noexcept { return data_view.size(); }

            /** True if ordered span of LHS is less than span of RHS */
            [[nodiscard]] bool operator()(const size_t lhs_offset, const size_t rhs_offset) const noexcept {
                const int_t the_size = data_view.size();

                for (size_t index = 0; index < the_size; ++index) {
                    const int_t lhs_index = (index + lhs_offset) % the_size;
                    const int_t rhs_index = (index + rhs_offset) % the_size;

                    if (data_view[lhs_index] < data_view[rhs_index]) {
                        return true;  // Left offset 'gap' strictly smaller than right offset 'gap'
                    } else if (data_view[lhs_index] > data_view[rhs_index]) {
                        return false; // Left offset 'gap' strictly larger than right offset 'gap'
                    }
                }
                return false; // Entire sequence is identical size
            }
        };

        [[nodiscard]] constexpr size_t operator()(const std::span<const int_t> data_view) const noexcept {
            ComparePermutationFunctor less{data_view};
            size_t optimal_index = 0;
            const size_t data_size = data_view.size();
            for (size_t alternative_index = 1; alternative_index < data_size; ++alternative_index) {
                if (less(alternative_index, optimal_index)) {
                    optimal_index = alternative_index;
                }
            }
            return optimal_index;
        }

        [[nodiscard]] constexpr inline size_t operator()(std::initializer_list<const int_t> init_list) const noexcept {
            return (*this)(std::span<const int_t>(init_list.begin(), init_list.size()));
        }

    };
 }