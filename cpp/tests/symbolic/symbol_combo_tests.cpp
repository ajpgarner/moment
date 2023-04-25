/**
 * symbol_combo_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/symbol_combo.h"
#include "symbolic/symbol_table.h"

#include "scenarios/imported/imported_matrix_system.h"

namespace Moment::Tests {
    TEST(Symbolic_SymbolCombo, CreateEmpty) {
        SymbolCombo empty{};
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.begin(), empty.end());
    }

    TEST(Symbolic_SymbolCombo, CreateThreeElems) {
        SymbolCombo threeElems{SymbolExpression{2, 13.0}, SymbolExpression{10, 100.0}, SymbolExpression{5, -23.0}};
        ASSERT_FALSE(threeElems.empty());
        ASSERT_EQ(threeElems.size(), 3);
        auto iter = threeElems.begin();
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[0]);
        EXPECT_EQ(iter->id, 2);
        EXPECT_EQ(iter->factor, 13.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[1]);
        EXPECT_EQ(iter->id, 5);
        EXPECT_EQ(iter->factor, -23.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[2]);
        EXPECT_EQ(iter->id, 10);
        EXPECT_EQ(iter->factor, 100.0);

        ++iter;
        ASSERT_EQ(iter, threeElems.end());
    }


    TEST(Symbolic_SymbolCombo, Create_Overlapped1) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0},
                                 SymbolExpression{2, 20.0}, SymbolExpression{3, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 50.0}, SymbolExpression{3, 40.0}};
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, Create_Overlapped2) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0},
                                 SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}};
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, Create_Overlapped3) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}, SymbolExpression{3, 50.0},
                                 SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}, SymbolExpression{3, 50.0}};
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedToZero) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{1, -10.0}};
        const SymbolCombo expected = SymbolCombo::Zero();
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedWithZero1) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{1, -10.0}, SymbolExpression{2, 20.0}};
        const SymbolCombo expected = SymbolCombo{{SymbolExpression{2, 20.0}}};
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedWithZero2) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, -20.0},
                                 SymbolExpression{2, 20.0}, SymbolExpression{3, 10.0}};
        const SymbolCombo expected = SymbolCombo{{SymbolExpression{1, 10.0}, SymbolExpression{3, 10.0}}};
        EXPECT_EQ(actual, expected);
    }

    TEST(Symbolic_SymbolCombo, CreateFromMap) {
        std::map<symbol_name_t, double> testMap{{2,  13.0},
                                                {10, 100.0},
                                                {5,  -23.0}};

        SymbolCombo threeElems{testMap};
        ASSERT_FALSE(threeElems.empty());
        ASSERT_EQ(threeElems.size(), 3);
        auto iter = threeElems.begin();
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[0]);
        EXPECT_EQ(iter->id, 2);
        EXPECT_EQ(iter->factor, 13.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[1]);
        EXPECT_EQ(iter->id, 5);
        EXPECT_EQ(iter->factor, -23.0);

        ++iter;
        ASSERT_NE(iter, threeElems.end());
        EXPECT_EQ(&(*iter), &threeElems[2]);
        EXPECT_EQ(iter->id, 10);
        EXPECT_EQ(iter->factor, 100.0);

        ++iter;
        ASSERT_EQ(iter, threeElems.end());
    }


    TEST(Symbolic_SymbolCombo, Equality) {
        SymbolCombo listA{SymbolExpression{2, 10.0}, SymbolExpression{5, 20.0}};
        SymbolCombo listB{SymbolExpression{2, 10.0}, SymbolExpression{5, 20.0}};
        SymbolCombo listC{SymbolExpression{2, 10.0}, SymbolExpression{10, 20.0}};
        SymbolCombo listD{SymbolExpression{2, 10.0}, SymbolExpression{10, 19.0}};
        SymbolCombo listE{SymbolExpression{2, 10.0}};
        SymbolCombo listF{SymbolExpression{2, 10.0}, SymbolExpression{5, 40.0}};

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

    TEST(Symbolic_SymbolCombo, Addition_NoOverlap) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{2, 20.0}};
        const SymbolCombo listB{SymbolExpression{3, 30.0}, SymbolExpression{4, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 20.0},
                                   SymbolExpression{3, 30.0}, SymbolExpression{4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_SymbolCombo, Addition_Interleaved) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{3, 30.0}};
        const SymbolCombo listB{SymbolExpression{2, 20.0}, SymbolExpression{4, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 20.0},
                                   SymbolExpression{3, 30.0}, SymbolExpression{4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_SymbolCombo, Addition_Overlapped1) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}};
        const SymbolCombo listB{SymbolExpression{2, 20.0}, SymbolExpression{3, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 50.0}, SymbolExpression{3, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_SymbolCombo, Addition_Overlapped2) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}};
        const SymbolCombo listB{SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_SymbolCombo, Addition_Overlapped3) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}, SymbolExpression{3, 50.0}};
        const SymbolCombo listB{SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}, SymbolExpression{3, 50.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_SymbolCombo, Addition_ToZero) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}};
        const SymbolCombo listB{SymbolExpression{1, -10.0}, SymbolExpression{2, -30.0}};
        const SymbolCombo expected = SymbolCombo::Zero();
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }


    TEST(Symbolic_SymbolCombo, SelfAddition) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{3, 30.0}};
        const SymbolCombo listB{SymbolExpression{2, 20.0}, SymbolExpression{4, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 20.0},
                                   SymbolExpression{3, 30.0}, SymbolExpression{4, 40.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list += listB;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_SymbolCombo, PostMultiply) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{3, 30.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{3, 90.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list *= 3;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_SymbolCombo, MultiplyFactor) {
        const SymbolCombo listA{SymbolExpression{1, 10.0}, SymbolExpression{3, 30.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{3, 90.0}};

        const auto listB = listA * 3;
        EXPECT_NE(listA, listB);
        EXPECT_EQ(listB, expected);
    }

    TEST(Symbolic_SymboCombo, IsHermitian) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo comboEmpty{};
        EXPECT_TRUE(comboEmpty.is_hermitian(symbols));

        const SymbolCombo combo_H_Id{SymbolExpression{1, 1.0}};
        EXPECT_TRUE(combo_H_Id.is_hermitian(symbols));

        const SymbolCombo combo_H_A{SymbolExpression{2, 1.0}};
        EXPECT_TRUE(combo_H_A.is_hermitian(symbols));

        const SymbolCombo combo_H_B_Bstar{SymbolExpression{3, 1.0}, SymbolExpression{3, 1.0, true}};
        EXPECT_TRUE(combo_H_B_Bstar.is_hermitian(symbols));

        const SymbolCombo combo_H_C_Cstar{SymbolExpression{4, 1.0}, SymbolExpression{4, 1.0, true}};
        EXPECT_TRUE(combo_H_C_Cstar.is_hermitian(symbols));

        const SymbolCombo combo_Id_B{SymbolExpression{1, 1.0}, SymbolExpression{3, 1.0}};
        EXPECT_FALSE(combo_Id_B.is_hermitian(symbols));

        const SymbolCombo combo_B{SymbolExpression{3, 1.0}};
        EXPECT_FALSE(combo_B.is_hermitian(symbols));

        const SymbolCombo combo_B_3Bstar{SymbolExpression{3, 1.0}, SymbolExpression{3, 2.0, true}};
        EXPECT_FALSE(combo_B_3Bstar.is_hermitian(symbols));

    }

    TEST(Symbolic_SymboCombo, Conjugate_Empty) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo comboEmpty{};
        const auto comboEmptyConj = comboEmpty.conjugate(symbols);
        EXPECT_EQ(comboEmpty, comboEmptyConj);
    }

    TEST(Symbolic_SymboCombo, Conjugate_Real) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{2, 2.0, false}};
        const SymbolCombo comboConjExp{SymbolExpression{2, 2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_SymboCombo, Conjugate_RealCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 2.0, false}};
        const SymbolCombo comboConjExp{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_SymboCombo, Conjugate_Imaginary) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{4, 2.0, false}};
        const SymbolCombo comboConjExp{SymbolExpression{4, -2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_SymboCombo, Conjugate_RealImaginaryCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{1, 1.0, false}, SymbolExpression{4, 2.0, false}};
        const SymbolCombo comboConjExp{SymbolExpression{1, 1.0, false}, SymbolExpression{4, -2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_SymboCombo, Conjugate_Complex) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{3, 2.0, false}};
        const SymbolCombo comboConjExp{SymbolExpression{3, 2.0, true}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_SymboCombo, Conjugate_ComplexCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo combo{SymbolExpression{3, 2.0, false}, SymbolExpression{3, 1.0, true}};
        const SymbolCombo comboConjExp{SymbolExpression{3, 1.0, false}, SymbolExpression{3, 2.0, true}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

}