/**
 * write_as_array.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once


#include "MatlabDataArray.hpp"

#include <concepts>
#include <stdexcept>
#include <vector>

namespace Moment::mex {

    template<std::integral output_t, typename iter_t>
    matlab::data::TypedArray<output_t>
    write_as_array(matlab::data::ArrayFactory& factory, iter_t iter, iter_t iter_end, const bool row_vector = true) {
        const ptrdiff_t size = std::distance(iter, iter_end);
        matlab::data::ArrayDimensions dimensions{1ULL, static_cast<size_t>(size)};
        if (!row_vector) {
            std::swap(dimensions[0], dimensions[1]);
        }
        auto output = factory.createArray<output_t>(std::move(dimensions));
        auto write_iter = output.begin();

        while(iter != iter_end) {
            *write_iter = static_cast<output_t>(*iter);
            ++write_iter;
            ++iter;
        }
        return output;
    }

}