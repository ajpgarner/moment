/**
 * utf_conversion_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/utf_conversion.h"

namespace Moment::Tests {


    TEST(Utilities_UTFConversion, UTF16to8_Empty) {
        const std::basic_string<char16_t> empty{};
        const std::string expected{};
        auto actual = UTF16toUTF8Convertor::convert(empty);
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF16toUTF8Convertor::convert_as_ascii(empty), expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorld) {
        const std::basic_string<char16_t> source{u"Hello world"};
        const std::string expected{"Hello world"};
        auto actual = UTF16toUTF8Convertor::convert(source);
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF16toUTF8Convertor::convert_as_ascii(source), expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorldInVietnamese) {
        const std::basic_string<char16_t> source{u"Ch\u00e0o th\u1ebf gi\u1edbi"}; // Chào thế giới
        const std::string expected{"Ch\xc3\xa0o th\xe1\xba\xbf gi\xe1\xbb\x9bi"};
        auto actual = UTF16toUTF8Convertor{}(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorldInChinese) {
        const std::basic_string<char16_t> source{u"\u4f60\u597d\u4e16\u754c\uff01"}; // 你好世界！
        const std::string expected{"\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81"};
        auto actual = UTF16toUTF8Convertor::convert(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_ReallyWeirdCharacters) {
        // The 'Hwair' https://en.wikipedia.org/wiki/Hwair
        const char16_t source_chars[] = {static_cast<char16_t>(0xd800), static_cast<char16_t>(0xdf48), 0x00};
        const std::basic_string<char16_t> source(source_chars);
        const char expected_chars[] = {static_cast<char>(0xf0), static_cast<char>(0x90),
                                       static_cast<char>(0x8d), static_cast<char>(0x88), 0x00};
        const std::string expected(expected_chars);
        auto actual = UTF16toUTF8Convertor::convert(source);
        EXPECT_EQ(actual, expected);
    }


    TEST(Utilities_UTFConversion, UTF16to8_ASCIISubset_HelloWorld) {
        const std::basic_string<char16_t> source{u"Hello world"};
        const std::string expected{"Hello world"};

        auto actual = UTF16toUTF8Convertor::convert_as_ascii(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_ASCIISubset_Mangled) {
        const std::basic_string<char16_t> source{u"Chào thế giới"};
        // Don't care about output, but must not crash.
        EXPECT_NO_THROW([[maybe_unused]] auto actual = UTF16toUTF8Convertor::convert_as_ascii(source));
    }


    TEST(Utilities_UTFConversion, UTF8to16_Empty) {
        const std::string empty{};
        const std::basic_string<char16_t> expected{};
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(empty);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF8toUTF16Convertor::convert_as_ascii(empty), expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorld) {
        const std::string source{"Hello world"};
        const std::basic_string<char16_t> expected{u"Hello world"};
        auto actual = UTF8toUTF16Convertor::convert(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF8toUTF16Convertor::convert_as_ascii(source), expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorldInVietnamese) {
        const std::string source{"Ch\xc3\xa0o th\xe1\xba\xbf gi\xe1\xbb\x9bi"}; // Chào thế giới
        const std::basic_string<char16_t> expected{u"Ch\u00e0o th\u1ebf gi\u1edbi"};
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor::convert(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorldInChinese) {
        const std::string source{"\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81"};
        const std::basic_string<char16_t> expected{u"\u4f60\u597d\u4e16\u754c\uff01"}; // 你好世界！
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor::convert(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ReallyWeirdCharacters) {
        // The 'Hwair' https://en.wikipedia.org/wiki/Hwair
        const char source_chars[] = {static_cast<char>(0xf0), static_cast<char>(0x90),
                                     static_cast<char>(0x8d), static_cast<char>(0x88), 0x00};
        const std::string source(source_chars);
        const char16_t expected_chars[] = {static_cast<char16_t>(0xd800), static_cast<char16_t>(0xdf48), 0x00};
        const std::basic_string<char16_t> expected(expected_chars);
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ASCIISubset_HelloWorld) {
        const std::string source{"Hello world"};
        const std::basic_string<char16_t> expected{u"Hello world"};

        auto actual = UTF8toUTF16Convertor::convert_as_ascii(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ASCIISubset_Mangled) {
        const std::string source{"Chào thế giới"};
        // Don't care about output, but must not crash:
        EXPECT_NO_THROW([[maybe_unused]] auto actual = UTF8toUTF16Convertor::convert_as_ascii(source));

    }


}