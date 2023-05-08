/**
 * utf_conversion.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "utf_conversion.h"

#include <cassert>

#include <algorithm>

namespace Moment {

    size_t UTF16toUTF8Convertor::size_in_utf8(const std::basic_string<char16_t> &wstring) {
        size_t output = 0;
        for (char16_t elem : wstring) {
            if (elem < 0x007f) {
                output += 1;
            } else if (elem <= 0x07FF) {
                output += 2;
            } else if ((elem >= 0xD800) && (elem <= 0xDfff)) {
                output += 2; // part of surrogate pair. If correctly encoded two will appear to contribute 4 in total.
            } else {
                output += 3;
            }
        }
        return output;
    }


    std::string UTF16toUTF8Convertor::convert_as_ascii(const std::basic_string<char16_t> &wstring) {
        const size_t strlen = wstring.size();
        std::string output(strlen, '\0');

        for (size_t idx = 0; idx < strlen; ++idx) {
            output[idx] = static_cast<char>(wstring[idx] & 0x7f);
        }

        return output;
    }

    std::string UTF16toUTF8Convertor::convert(const std::basic_string<char16_t>& wstring) {
        const char16_t* src_ptr = wstring.data();
        const char16_t* const src_end = src_ptr + wstring.size();

        // Quick convert for ASCII sequence
        const size_t out_capacity = UTF16toUTF8Convertor::size_in_utf8(wstring);

        const bool is_ascii = wstring.size() == out_capacity;
        if (is_ascii) {
            return convert_as_ascii(wstring);
        }


        // Otherwise, do proper conversion
        size_t out_index = 0;
        std::string output(out_capacity, '\0');


        while (src_ptr < src_end) {
            const char16_t src = *src_ptr;

            if (src <= 0x007F) {
                // Source UTF16 character can be represented as single UTF8 character
                output[out_index++] = static_cast<char>(src & 0x007F);
                ++src_ptr;
                continue;
            }

            // Is source code not directly encoded?
            const bool is_special_utf16 = (src & 0xF800) == 0xD800;

            // Direct encode in UTF16 -> 2 to 3 bytes in UTF8
            if (!is_special_utf16) {
                if (src <= 0x07FF) {

                    // 0000 0xxx xxyy yyyy -> 110x xxxx, 10yy yyyy
                    const auto hi = static_cast<char>( 0xc0 | ((0x07c0 & src) >> 6));
                    const auto lo = static_cast<char>( 0x80 | (0x003f & src));
                    output[out_index++] = hi;
                    output[out_index++] = lo;

                } else { // three bytes
                    // xxxx yyyy yyzz zzzz -> 1110 xxxx, 10yy yyyy, 10zz zzzz
                    const auto hi = static_cast<char>(0xe0 | ((0xf000 & src) >> 12));
                    const auto mid = static_cast<char>(0x80 | (0x0fc0 & src) >> 6);
                    const auto lo = static_cast<char>(0x80 | (0x003f & src));
                    output[out_index++] = hi;
                    output[out_index++] = mid;
                    output[out_index++] = lo;
                }
                ++src_ptr;
                continue;
            }

            // Process pair...
            // Do we have a proper surrogate pair?
            if (src_ptr+1 < src_end) {
                const char16_t surrogate_low = *(src_ptr+1);
                if (((src & 0xFC00) == 0xD800) && ((surrogate_low & 0xFC00) == 0xDC00)) {

                    //U' = aabb bbbb cccc ccdd dddd    // U - 0x10000 <- only affects  17-22nd bit
                    //W1 = 1101 10aa bbbb bbcc      // 0xD800 + [yy yy]yy yyyy
                    //W2 = 1101 11cc cc dd dddd      // 0xDC00 + xx xxxx xxxx
                    //
                    // As UTF8: 2+6+6+6 -> 20 bit
                    //  1111 0[0]aa, 10bb bbbb, 10cc cccc, 10dd dddd

                    const char16_t surrogate_high = (src & 0x03FF) + 0x0040; // Correct for 0x10000 offset of code point

                    const auto b0 = static_cast<char>( 0xf0 | ((surrogate_high & 0x0300) >> 8)); // 2 bits to first char
                    const auto b1 = static_cast<char>( 0x80 | ((surrogate_high & 0x00fc) >> 2)); // 6 bits to next char
                    const auto b2 = static_cast<char>( 0x80 | ((surrogate_high & 0x0003) << 4) // 2 from high
                                                            | ((surrogate_low & 0x03c0) >> 6));  // 4 from low
                    const auto b3 = static_cast<char>( 0x80 | (surrogate_low & 0x003f)); // 6 from low

                    output[out_index++] = b0;
                    output[out_index++] = b1;
                    output[out_index++] = b2;
                    output[out_index++] = b3;

                    src_ptr += 2;
                    continue;
                }
            }

            // Unmatched surrogate. For now, ignore.
            output[out_index++] = 0;
            output[out_index++] = 0;
            ++src_ptr;
        }

        return output;
    }

    size_t UTF8toUTF16Convertor::size_in_utf16(const std::string& string) {
        size_t output = 0;
        const size_t strlen = string.size();

        size_t idx = 0;
        while (idx < strlen) {
            const char elem = string[idx];
            if (!(elem & 0x80))  {
                output += 1;
                idx += 1; // 1 byte in UTF8
                continue;
            }

            if ((elem & 0xe0) == 0xc0) {
                output += 1;
                idx += 2; // extra consume, 2 bytes in UTF8
                continue;
            }

            if ((elem & 0xf0) == 0xe0) {
                output += 1;
                idx += 3; // extra consume, 3 bytes in UTF8
                continue;

            }
            if ((elem & 0xf8) == 0xf0) {
                output += 2;
                idx += 4; // extra consume, 4 bytes in UTF8
                continue;
            }

            output +=1;
            ++idx;
        }

        return output;
    }


    std::basic_string<char16_t> UTF8toUTF16Convertor::convert_as_ascii(const std::string& string) {
        const size_t strlen = string.size();
        std::basic_string<char16_t> output(strlen, 0x0000);

        for (size_t idx = 0; idx < strlen; ++idx) {
            output[idx] = static_cast<char16_t>(string[idx] & 0x7f);
        }

        return output;
    }

    std::basic_string<char16_t> UTF8toUTF16Convertor::convert(const std::string& string) {
        const char* src_ptr = string.data();
        const char* const src_end = src_ptr + string.size();

        // Quick convert for ASCII sequence
        const size_t out_capacity = UTF8toUTF16Convertor::size_in_utf16(string);

        const bool is_ascii = string.size() == out_capacity;
        if (is_ascii) {
            return UTF8toUTF16Convertor::convert_as_ascii(string);
        }


        // Otherwise, do proper conversion
        size_t out_index = 0;
        std::basic_string<char16_t> output(out_capacity, 0x0000);


        while (src_ptr < src_end) {
            const char src = *src_ptr;

            if (!(src & 0x80))  {
                // Source UTF8 character can be directly represented as single UTF16 character
                output[out_index++] = static_cast<char16_t>(src);
                ++src_ptr;
                continue;
            }

            if ((src & 0xe0) == 0xc0) {
                // Two byte UTF -> 1 word UTF16 expected [U+0080 to U+07FF]
                // 110xxxxx,	10yyyyyy -> 0x00xxxx xxyy yyyy
                if (src_ptr+1 >= src_end) {
                    return output;
                }
                const char lo = *(src_ptr+1);
                output[out_index++] = static_cast<char16_t>(((0x1f & src) << 6) | (0x3f & lo));

                src_ptr += 2;
                continue;
            }

            if ((src & 0xf0) == 0xe0) {
                // Three byte UTF8 -> 1 word UTF16
                // 	1110xxxx, 10yyyyyy, 10zzzzzz ->
                if (src_ptr + 2 >= src_end) {
                    return output;
                }
                const char hi = src & 0x0f;
                const char mid = *(src_ptr+1) & 0x3f;
                const char lo = *(src_ptr+2) & 0x3f;
                const auto code_point =  static_cast<char16_t>((hi << 12) | (mid << 6) | lo);

                output[out_index++] = code_point;

                src_ptr += 3;
                continue;
            }
            if ((src & 0xf8) == 0xf0) {
                // Four bytes expected
                // 	111100ww, 10xxxxxx, 10yyyyyy, 10zzzzzz ->
                if (src_ptr+3 >= src_end) {
                    return output;
                }

                const auto b1 = src & 0x03; // 2
                const auto b2 = *(src_ptr+1) & 0x3f; // 6
                const auto b3 = *(src_ptr+2) & 0x3f; // 6
                const auto b4 = *(src_ptr+3) & 0x3f; // 6

                const auto hi_surrogate = static_cast<char16_t>((0xd800 | (b1 << 8) | (b2 << 2) | ((b3&0x30) >> 4))
                                                                - 0x0040); // - 0x10000, as viewed on high surrogate.
                const auto lo_surrogate = static_cast<char16_t>(0xdc00 | ((b3 & 0x0f) << 6) | b4);

                output[out_index++] = hi_surrogate;
                output[out_index++] = lo_surrogate;

                src_ptr += 4;
                continue;
            }

            // Should not occur
            ++src_ptr;
        }

        assert(out_index <= out_capacity);

        return output;
    }

}