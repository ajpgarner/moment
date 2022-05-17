/**
 * wrapped_arg_range.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Due to matlab's weird usage of templates, matlab::mex::ArgumentList can seemingly only be used in one file without
 * causing linker errors (due to non-inline functions defined in the header file that contains the full definition of
 * MexIORange).
 *
 * Thus, the WrappedArgRange class basically implements the same behaviour as matlab::mex::ArgumentList, with the added
 * bonus of actually compiling and linking when used in more than one compilation unit.
 */
#pragma once

#include "MatlabDataArray.hpp"
#include <cassert>

namespace NPATK::mex {
    class WrappedArgRange {
    public:
        using iter_type = std::vector<matlab::data::Array>::iterator;

    private:
        iter_type i_start, i_end;
        size_t elem_count;

    public:
        constexpr WrappedArgRange(iter_type first, iter_type end)
                : i_start(first), i_end(end), elem_count{static_cast<size_t>(std::distance(first, end))} {}

        WrappedArgRange(const WrappedArgRange &rhs) = default;

        [[nodiscard]] constexpr size_t size() const { return elem_count; }

        [[nodiscard]] constexpr iter_type begin() { return i_start; }

        [[nodiscard]] constexpr iter_type end() { return i_end; }

        constexpr matlab::data::Array &operator[](iter_type::difference_type elem) {
            assert((elem >= 0) && (elem < this->elem_count));
            return *(i_start + elem);
        }
    };
}