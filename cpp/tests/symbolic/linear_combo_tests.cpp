/**
 * linear_combo_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "symbolic/linear_combo.h"

namespace NPATK::Tests {
    TEST(Symbolic_LinearCombo, CreateEmpty) {
        LinearCombo empty{};
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.begin(), empty.end());
    }

    TEST(Symbolic_LinearCombo, CreateThreeElems) {
        LinearCombo threeElems{{2, 13.0}, {10, 100.0}, {5, -23.0}};
        ASSERT_FALSE(threeElems.empty());
        ASSERT_EQ(threeElems.size(), 3);
        auto iter = threeElems.begin();
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[0]);
        EXPECT_EQ(iter->first, 2);
        EXPECT_EQ(iter->second, 13.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[1]);
        EXPECT_EQ(iter->first, 5);
        EXPECT_EQ(iter->second, -23.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[2]);
        EXPECT_EQ(iter->first, 10);
        EXPECT_EQ(iter->second, 100.0);

        ++iter;
        ASSERT_EQ(iter, threeElems.end());
    }

    TEST(Symbolic_LinearCombo, Equality) {
        LinearCombo listA{{2, 10.0}, {5, 20.0}};
        LinearCombo listB{{2, 10.0}, {5, 20.0}};
        LinearCombo listC{{2, 10.0}, {10, 20.0}};
        LinearCombo listD{{2, 10.0}, {10, 19.0}};
        LinearCombo listE{{2, 10.0}};
        LinearCombo listF{{2, 10.0}, {5, 40.0}};

        EXPECT_TRUE(listA == listB);
        EXPECT_TRUE(listB == listA);
        EXPECT_TRUE(listA != listC);
        EXPECT_TRUE(listA != listD);
        EXPECT_TRUE(listA != listE);
        EXPECT_TRUE(listA != listF);

        EXPECT_FALSE(listA != listB);
        EXPECT_FALSE(listB != listA);
        EXPECT_FALSE(listA == listC);
        EXPECT_FALSE(listA == listD);
        EXPECT_FALSE(listA == listE);
        EXPECT_FALSE(listA == listF);
    }

    TEST(Symbolic_LinearCombo, Addition_NoOverlap) {
        const LinearCombo listA{{1, 10.0}, {2, 20.0}};
        const LinearCombo listB{{3, 30.0}, {4, 40.0}};
        const LinearCombo expected{{1, 10.0}, {2, 20.0},{3, 30.0}, {4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_LinearCombo, Addition_Interleaved) {
        const LinearCombo listA{{1, 10.0}, {3, 30.0}};
        const LinearCombo listB{{2, 20.0}, {4, 40.0}};
        const LinearCombo expected{{1, 10.0}, {2, 20.0},{3, 30.0}, {4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_LinearCombo, Addition_Overlapped1) {
        const LinearCombo listA{{1, 10.0}, {2, 30.0}};
        const LinearCombo listB{{2, 20.0}, {3, 40.0}};
        const LinearCombo expected{{1, 10.0}, {2, 50.0}, {3, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_LinearCombo, Addition_Overlapped2) {
        const LinearCombo listA{{1, 10.0}, {2, 30.0}};
        const LinearCombo listB{{1, 20.0}, {2, 40.0}};
        const LinearCombo expected{{1, 30.0}, {2, 70.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_LinearCombo, Addition_Overlapped3) {
        const LinearCombo listA{{1, 10.0}, {2, 30.0}, {3, 50.0}};
        const LinearCombo listB{{1, 20.0}, {2, 40.0}};
        const LinearCombo expected{{1, 30.0}, {2, 70.0}, {3, 50.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }


    TEST(Symbolic_LinearCombo, SelfAddition) {
        const LinearCombo listA{{1, 10.0}, {3, 30.0}};
        const LinearCombo listB{{2, 20.0}, {4, 40.0}};
        const LinearCombo expected{{1, 10.0}, {2, 20.0},{3, 30.0}, {4, 40.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list += listB;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_LinearCombo, PostMultiply) {
        const LinearCombo listA{{1, 10.0}, {3, 30.0}};
        const LinearCombo expected{{1, 30.0}, {3, 90.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list *= 3;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_LinearCombo, MultiplyFactor) {
        const LinearCombo listA{{1, 10.0}, {3, 30.0}};
        const LinearCombo expected{{1, 30.0}, {3, 90.0}};

        const auto listB = listA * 3;
        EXPECT_NE(listA, listB);
        EXPECT_EQ(listB, expected);
    }

}