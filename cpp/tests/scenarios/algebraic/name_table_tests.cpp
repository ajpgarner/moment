/**
 * name_table_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/name_table.h"

#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Scenarios_Algebraic_NameTable, Validate_Empty) {
        std::string empty{};
        auto result = NameTable::validate_name(empty);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{empty}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_StartsWithNumber) {
        std::string bad_string{"0bad"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_StartsWithUnderscore) {
        std::string bad_string{"_bad"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_OnlyNumber1) {
        std::string bad_string{"0"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_OnlyNumber2) {
        std::string bad_string{"00"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_NoSpace) {
        std::string bad_string{"X Y"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_BadChar) {
        std::string bad_string{"X'"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_LowerChar) {
        std::string good_string{"x"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_UpperChar) {
        std::string good_string{"X"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_Word) {
        std::string good_string{"Cake"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_Snake) {
        std::string good_string{"test_word"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_WithNumbers) {
        std::string good_string{"X1"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_DuplicateNames) {
        std::vector<std::string> names{"X", "Y", "X"};
        EXPECT_THROW(NameTable(std::move(names)), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_XYZ) {
        NameTable names{std::vector<std::string>{"X", "Y", "Z"}};
        EXPECT_EQ(names[0], "X");
        EXPECT_EQ(names[1], "Y");
        EXPECT_EQ(names[2], "Z");
    }

    TEST(Scenarios_Algebraic_NameTable, Find_XYZ) {
        AlgebraicPrecontext apc{3};
        NameTable names{std::vector<std::string>{"X", "Y", "Z"}};

        EXPECT_EQ(names.find(apc, "X"), 0);
        EXPECT_EQ(names.find(apc, "Y"), 1);
        EXPECT_EQ(names.find(apc, "Z"), 2);
        EXPECT_EQ(names.find(apc, "X*"), 0);
        EXPECT_EQ(names.find(apc, "Y*"), 1);
        EXPECT_EQ(names.find(apc, "Z*"), 2);
        EXPECT_THROW(names.find(apc, "A"), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Find_XYZ_NonHermitian) {
        AlgebraicPrecontext apc{3, false};
        NameTable names{std::vector<std::string>{"X", "Y", "Z"}};

        EXPECT_EQ(names.find(apc, "X"), 0);
        EXPECT_EQ(names.find(apc, "Y"), 1);
        EXPECT_EQ(names.find(apc, "Z"), 2);
        EXPECT_EQ(names.find(apc, "X*"), 3);
        EXPECT_EQ(names.find(apc, "Y*"), 4);
        EXPECT_EQ(names.find(apc, "Z*"), 5);
        EXPECT_THROW(names.find(apc, "A"), std::invalid_argument);
        EXPECT_THROW(names.find(apc, "A*"), std::invalid_argument);
        EXPECT_THROW(names.find(apc, "X**"), std::invalid_argument);
    }
}