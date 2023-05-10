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
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{empty}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_StartsWithNumber) {
        std::string bad_string{"0bad"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_StartsWithUnderscore) {
        std::string bad_string{"_bad"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_OnlyNumber1) {
        std::string bad_string{"0"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_OnlyNumber2) {
        std::string bad_string{"00"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_NoSpace) {
        std::string bad_string{"X Y"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_BadChar) {
        std::string bad_string{"X'"};
        auto result = NameTable::validate_name(bad_string);
        EXPECT_TRUE(result.has_value());
        EXPECT_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{bad_string}), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_LowerChar) {
        std::string good_string{"x"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_UpperChar) {
        std::string good_string{"X"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_Word) {
        std::string good_string{"Cake"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_Snake) {
        std::string good_string{"test_word"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Validate_WithNumbers) {
        std::string good_string{"X1"};
        auto result = NameTable::validate_name(good_string);
        EXPECT_FALSE(result.has_value());
        EXPECT_NO_THROW(NameTable(AlgebraicPrecontext{1}, std::vector{good_string}));
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_DuplicateNames) {
        std::vector<std::string> names{"X", "Y", "X"};
        EXPECT_THROW(NameTable(AlgebraicPrecontext{3}, std::move(names)), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_MiscountNames) {
        std::vector<std::string> names{"X", "Y", "Z"};
        EXPECT_THROW(NameTable(AlgebraicPrecontext{2}, std::move(names)), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_DefaultEmpty) {
        NameTable names{};
        EXPECT_EQ(names.operator_count, 0);
    }

    TEST(Scenarios_Algebraic_NameTable, Construct_XYZ) {
        NameTable names{AlgebraicPrecontext{3}, std::vector<std::string>{"X", "Y", "Z"}};
        EXPECT_EQ(names[0], "X");
        EXPECT_EQ(names[1], "Y");
        EXPECT_EQ(names[2], "Z");
    }


    TEST(Scenarios_Algebraic_NameTable, Construct_XYZ_InitList) {
        NameTable names{AlgebraicPrecontext{3}, {"X", "Y", "Z"}};
        EXPECT_EQ(names[0], "X");
        EXPECT_EQ(names[1], "Y");
        EXPECT_EQ(names[2], "Z");
    }

    TEST(Scenarios_Algebraic_NameTable, Find_XYZ) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        NameTable names{apc, std::vector<std::string>{"X", "Y", "Z"}};

        EXPECT_EQ(names.find("X"), 0);
        EXPECT_EQ(names.find("Y"), 1);
        EXPECT_EQ(names.find("Z"), 2);
        EXPECT_EQ(names.find("X*"), 0);
        EXPECT_EQ(names.find("Y*"), 1);
        EXPECT_EQ(names.find("Z*"), 2);
        EXPECT_THROW(auto x = names.find("A"), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Find_XYZ_NonHermitian) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        NameTable names{apc, std::vector<std::string>{"X", "Y", "Z"}};

        EXPECT_EQ(names.find("X"), 0);
        EXPECT_EQ(names.find("Y"), 1);
        EXPECT_EQ(names.find("Z"), 2);
        EXPECT_EQ(names.find("X*"), 3);
        EXPECT_EQ(names.find("Y*"), 4);
        EXPECT_EQ(names.find("Z*"), 5);
        EXPECT_THROW(auto x = names.find("A"), std::invalid_argument);
        EXPECT_THROW(auto y = names.find("A*"), std::invalid_argument);
        EXPECT_THROW(auto z = names.find("X**"), std::invalid_argument);
    }

    TEST(Scenarios_Algebraic_NameTable, Find_XYZ_NonHermitianInterleaved) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Interleaved};
        NameTable names{apc, std::vector<std::string>{"X", "Y", "Z"}};

        EXPECT_EQ(names.find("X"), 0);
        EXPECT_EQ(names.find("Y"), 2);
        EXPECT_EQ(names.find("Z"), 4);
        EXPECT_EQ(names.find("X*"), 1);
        EXPECT_EQ(names.find("Y*"), 3);
        EXPECT_EQ(names.find("Z*"), 5);
        EXPECT_THROW(auto x = names.find("A"), std::invalid_argument);
        EXPECT_THROW(auto y = names.find("A*"), std::invalid_argument);
        EXPECT_THROW(auto z = names.find("X**"), std::invalid_argument);
    }
}