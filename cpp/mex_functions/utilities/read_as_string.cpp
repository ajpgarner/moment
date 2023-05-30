/**
 * read_as_string.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_as_string.h"
#include "utilities/utf_conversion.h"

namespace Moment::mex {
    std::optional<std::string> read_as_utf8(matlab::data::Array input) {
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            matlab::data::CharArray char_name = input;
            return char_name.toAscii();
        }

        if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            matlab::data::TypedArray<matlab::data::MATLABString> the_str_array{std::move(input)};
            if (the_str_array.getNumberOfElements() < 1) {
                return std::nullopt;
            }

            auto mls = *(the_str_array.cbegin());
            return UTF16toUTF8Convertor{}(mls);
        }

        return std::nullopt;
    }

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

        return std::nullopt;
    }


}
