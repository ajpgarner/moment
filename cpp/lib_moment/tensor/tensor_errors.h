/**
 * tensor_errors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <concepts>
#include <stdexcept>
#include <string>

namespace Moment::errors {
    class bad_tensor : public std::runtime_error {
    public:
        explicit bad_tensor(const std::string& what) noexcept : std::runtime_error(what) { }
    };

    class bad_tensor_index : public bad_tensor {
    public:
        explicit bad_tensor_index(const std::string& what) noexcept : bad_tensor(what) { }

        static bad_tensor_index offset_out_of_range(const std::string& indexExpr, const std::string& maxExpr);

        template<std::integral int_t>
        inline static bad_tensor_index offset_out_of_range(int_t index, int_t max) {
            return offset_out_of_range(std::to_string(index), std::to_string(max));
        }


        static bad_tensor_index index_out_of_range(const std::string& dimensionExpr,
                                                   const std::string& indexExpr, const std::string& maxExpr);

        template<std::integral int_t>
        inline static bad_tensor_index index_out_of_range(size_t dimension, int_t index, int_t max) {
            return index_out_of_range(std::to_string(dimension), std::to_string(index), std::to_string(max));
        }

        static bad_tensor_index bad_dimension_count(size_t count, size_t max);

        static bad_tensor_index wrong_order(size_t d, const std::string& minExpr, const std::string& maxExpr);

        template<std::integral int_t>
        inline static bad_tensor_index wrong_order(size_t d, int_t min, int_t max) {
            return wrong_order(d, std::to_string(min), std::to_string(max));
        }
    };
}