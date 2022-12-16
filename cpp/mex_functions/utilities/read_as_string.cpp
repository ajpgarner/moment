/**
 * read_as_string.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "read_as_string.h"

namespace Moment::mex {

    std::optional<std::basic_string<char16_t>> read_as_utf16(matlab::data::Array input) {
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            matlab::data::CharArray char_name = input;
            return char_name.toUTF16();
        }

        if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            matlab::data::TypedArray<matlab::data::MATLABString> the_str_array{std::move(input)};
            if (the_str_array.getNumberOfElements() < 1) {
                return {};
            }

            auto mls = *(the_str_array.cbegin());
            return mls;
        }

        return {};
    }
}
