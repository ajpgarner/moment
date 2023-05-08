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
        std::basic_string<char16_t> empty{};
        std::string expected{};
        auto actual = UTF16toUTF8Convertor{}(empty);
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF16toUTF8Convertor::convert_as_ascii(empty), expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorld) {
        std::basic_string<char16_t> source{u"Hello world"};
        std::string expected{"Hello world"};
        auto actual = UTF16toUTF8Convertor{}(source);
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF16toUTF8Convertor::convert_as_ascii(source), expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorldInVietnamese) {
        std::basic_string<char16_t> source{u"Chào thế giới"};
        std::string expected{"Chào thế giới"};
        auto actual = UTF16toUTF8Convertor{}(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_HelloWorldInChinese) {
        std::basic_string<char16_t> source{u"你好世界！"};
        std::string expected{"你好世界！"};
        auto actual = UTF16toUTF8Convertor{}(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_ReallyWeirdCharacters) {
        // The 'Hwair' https://en.wikipedia.org/wiki/Hwair
        const char16_t source_chars[] = {static_cast<char16_t>(0xd800), static_cast<char16_t>(0xdf48), 0x00};
        std::basic_string<char16_t> source(source_chars);
        const char expected_chars[] = {static_cast<char>(0xf0), static_cast<char>(0x90),
                                       static_cast<char>(0x8d), static_cast<char>(0x88), 0x00};
        std::string expected(expected_chars);
        auto actual = UTF16toUTF8Convertor{}(source);
        EXPECT_EQ(actual, expected);
    }


    TEST(Utilities_UTFConversion, UTF16to8_ASCIISubset_HelloWorld) {
        std::basic_string<char16_t> source{u"Hello world"};
        std::string expected{"Hello world"};

        auto actual = UTF16toUTF8Convertor::convert_as_ascii(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF16to8_ASCIISubset_Mangled) {
        std::basic_string<char16_t> source{u"Chào thế giới"};
        // Don't care about output, but must not crash.
        EXPECT_NO_THROW([[maybe_unused]] auto actual = UTF16toUTF8Convertor::convert_as_ascii(source));
    }


    TEST(Utilities_UTFConversion, UTF8to16_Empty) {
        std::string empty{};
        std::basic_string<char16_t> expected{};
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(empty);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF8toUTF16Convertor::convert_as_ascii(empty), expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorld) {
        std::string source{"Hello world"};
        std::basic_string<char16_t> expected{u"Hello world"};
        auto actual = UTF8toUTF16Convertor{}(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(UTF8toUTF16Convertor::convert_as_ascii(source), expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorldInVietnamese) {
        std::string source{"Chào thế giới"};
        std::basic_string<char16_t> expected{u"Chào thế giới"};
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_HelloWorldInChinese) {
        std::string source{"你好世界！"};
        std::basic_string<char16_t> expected{u"你好世界！"};
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ReallyWeirdCharacters) {
        // The 'Hwair' https://en.wikipedia.org/wiki/Hwair
        const char source_chars[] = {static_cast<char>(0xf0), static_cast<char>(0x90),
                                     static_cast<char>(0x8d), static_cast<char>(0x88), 0x00};
        std::string source(source_chars);
        const char16_t expected_chars[] = {static_cast<char16_t>(0xd800), static_cast<char16_t>(0xdf48), 0x00};
        std::basic_string<char16_t> expected(expected_chars);
        std::basic_string<char16_t> actual = UTF8toUTF16Convertor{}(source);
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ASCIISubset_HelloWorld) {
        std::string source{"Hello world"};
        std::basic_string<char16_t> expected{u"Hello world"};

        auto actual = UTF8toUTF16Convertor::convert_as_ascii(source);
        EXPECT_EQ(actual, expected);
    }

    TEST(Utilities_UTFConversion, UTF8to16_ASCIISubset_Mangled) {
        std::string source{"Chào thế giới"};
        // Don't care about output, but must not crash:
        EXPECT_NO_THROW([[maybe_unused]] auto actual = UTF8toUTF16Convertor::convert_as_ascii(source));

    }


}