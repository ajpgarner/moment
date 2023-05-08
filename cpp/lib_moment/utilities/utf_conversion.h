/**
 * utf_conversion.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <string>

namespace Moment {

    /**
     * Non-streaming convertor from UTF-16 encoded strings to UTF-8 encoded strings.
     */
    class UTF16toUTF8Convertor {
    public:
        UTF16toUTF8Convertor() = default;

        /** Convert only first 7 bits of each char16, truncating the rest (i.e. treating the input as ASCII).
         * This will yield nonsense if a non-ASCII input is supplied. */
        [[nodiscard]] static std::string convert_as_ascii(const std::basic_string<char16_t>& wstring);

        /** Convert properly */
        [[nodiscard]] static std::string convert(const std::basic_string<char16_t>& wstring);

        /** Calculate how many bytes of UTF-8 are required to encode supplied UTF-16 string */
        [[nodiscard]] static size_t size_in_utf8(const std::basic_string<char16_t>& wstring);

        /** Convert UTF-16 encoded sequence into UTF-8 encoded sequence */
        [[nodiscard]] inline std::string operator()(const std::basic_string<char16_t>& wstring) const {
            return UTF16toUTF8Convertor::convert(wstring);
        }

    };


    /**
     * Non-streaming convertor from UTF-8 encoded strings to UTF-16 encoded strings.
     */
    class UTF8toUTF16Convertor {
    public:
        UTF8toUTF16Convertor() = default;

        /** Convert input byte by byte (i.e. treating as ASCII).
         * This will yield nonsense if a non-ASCII input is supplied. */
        [[nodiscard]] static std::basic_string<char16_t> convert_as_ascii(const std::string& string);

        /** Convert UTF-8 encoded sequence into UTF-16 encoded sequence. */
        [[nodiscard]] static std::basic_string<char16_t> convert(const std::string& wstring);

        /** Calculate how many (wide) chars of UTF-16 are required to encode supplied UTF-8 string. */
        [[nodiscard]] static size_t size_in_utf16(const std::string& string);

        /** Convert UTF-8 encoded sequence into UTF-16 encoded sequence. */
        [[nodiscard]] inline std::basic_string<char16_t> operator()(const std::string& wstring) const {
            return UTF8toUTF16Convertor::convert(wstring);
        }

    };



}
