/**
 * symbol_combo_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/symbol_combo.h"
#include "symbolic/symbol_table.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/imported/imported_matrix_system.h"


namespace Moment::Tests {
    TEST(Symbolic_SymbolCombo, Create_Empty) {
        SymbolCombo empty{};
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.begin(), empty.end());
        EXPECT_TRUE(empty.is_monomial());
        EXPECT_TRUE(empty.real_factors());
        EXPECT_EQ(empty.first_id(), 0);
        EXPECT_EQ(empty.last_id(), 0);
    }

    TEST(Symbolic_SymbolCombo, Create_Scalar) {
        SymbolCombo scalar = SymbolCombo::Scalar(2.5);
        EXPECT_FALSE(scalar.empty());
        ASSERT_EQ(scalar.size(), 1);
        EXPECT_TRUE(scalar.is_monomial());
        EXPECT_TRUE(scalar.real_factors());
        EXPECT_EQ(*scalar.begin(), SymbolExpression(1, 2.5));

        EXPECT_EQ(scalar.first_id(), 1);
        EXPECT_EQ(scalar.last_id(), 1);

    }
    TEST(Symbolic_SymbolCombo, Create_ComplexScalar) {
        SymbolCombo scalar = SymbolCombo::Scalar({2.5, 1.0});
        EXPECT_FALSE(scalar.empty());
        ASSERT_EQ(scalar.size(), 1);
        EXPECT_TRUE(scalar.is_monomial());
        EXPECT_FALSE(scalar.real_factors());
        EXPECT_EQ(*scalar.begin(), SymbolExpression(1, {2.5, 1.0}));

        EXPECT_EQ(scalar.first_id(), 1);
        EXPECT_EQ(scalar.last_id(), 1);

    }

    TEST(Symbolic_SymbolCombo, Create_OneElem) {
        SymbolCombo one_elem{SymbolExpression{13, -2.0}};
        EXPECT_FALSE(one_elem.empty());
        ASSERT_EQ(one_elem.size(), 1);
        EXPECT_TRUE(one_elem.is_monomial());
        EXPECT_TRUE(one_elem.real_factors());
        EXPECT_EQ(*one_elem.begin(), SymbolExpression(13, -2.0));

        EXPECT_EQ(one_elem.first_id(), 13);
        EXPECT_EQ(one_elem.last_id(), 13);
    }

    TEST(Symbolic_SymbolCombo, Create_ThreeElems) {
        SymbolCombo threeElems{SymbolExpression{2, 13.0}, SymbolExpression{10, 100.0}, SymbolExpression{5, -23.0}};
        ASSERT_FALSE(threeElems.empty());
        ASSERT_EQ(threeElems.size(), 3);
        EXPECT_TRUE(threeElems.real_factors());

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

        EXPECT_FALSE(threeElems.is_monomial());
        EXPECT_EQ(threeElems.first_id(), 2);
        EXPECT_EQ(threeElems.last_id(), 10);
    }

    TEST(Symbolic_SymbolCombo, Create_InitListZero) {
        SymbolCombo empty{SymbolExpression{0, 1.0}};
        EXPECT_TRUE(empty.empty()) << empty;
        EXPECT_EQ(empty.size(), 0)  << empty;
        EXPECT_EQ(empty.begin(), empty.end())  << empty;
        EXPECT_TRUE(empty.is_monomial()) << empty;
    }


    TEST(Symbolic_SymbolCombo, Create_Overlapped1) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0},
                                 SymbolExpression{2, 20.0}, SymbolExpression{3, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 10.0}, SymbolExpression{2, 50.0}, SymbolExpression{3, 40.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_Overlapped2) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0},
                                 SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_Overlapped3) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, 30.0}, SymbolExpression{3, 50.0},
                                 SymbolExpression{1, 20.0}, SymbolExpression{2, 40.0}};
        const SymbolCombo expected{SymbolExpression{1, 30.0}, SymbolExpression{2, 70.0}, SymbolExpression{3, 50.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedToZero) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{1, -10.0}};
        const SymbolCombo expected = SymbolCombo::Zero();
        EXPECT_EQ(actual, expected);
        EXPECT_TRUE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedWithZero1) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{1, -10.0}, SymbolExpression{2, 20.0}};
        const SymbolCombo expected = SymbolCombo{{SymbolExpression{2, 20.0}}};
        EXPECT_EQ(actual, expected);
        EXPECT_TRUE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_OverlappedWithZero2) {
        const SymbolCombo actual{SymbolExpression{1, 10.0}, SymbolExpression{2, -20.0},
                                 SymbolExpression{2, 20.0}, SymbolExpression{3, 10.0}};
        const SymbolCombo expected = SymbolCombo{{SymbolExpression{1, 10.0}, SymbolExpression{3, 10.0}}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_FromMap) {
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

        EXPECT_FALSE(threeElems.is_monomial());
    }


    TEST(Symbolic_SymbolCombo, Create_FromExpr) {
        const SymbolExpression expr{5, -2.0, true};
        const SymbolCombo combo{expr};
        ASSERT_EQ(combo.size(), 1);
        EXPECT_EQ(*combo.begin(), SymbolExpression(5, -2.0, true));
        EXPECT_TRUE(combo.is_monomial());
    }

    TEST(Symbolic_SymbolCombo, Create_FromExprZero) {
        const SymbolExpression expr{0, 1.0};
        const SymbolCombo combo(expr); // <- can't use {} as this will call init_list c'tor!!
        ASSERT_EQ(combo.size(), 0);
        EXPECT_TRUE(combo.is_monomial());
    }


    TEST(Symbolic_SymbolCombo, CopyConstruct_Empty) {
        const SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyConstruct_Scalar) {
        const SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyConstruct_Polynomial) {
        const SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Empty_OverEmpty) {
        const SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo::Zero();
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Empty_OverScalar) {
        const SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo::Scalar(13.37);
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Empty_OverPolynomial) {
        const SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Scalar_OverScalar) {
        const SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test = SymbolCombo::Scalar(0.2);
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Scalar_OverPolynomial) {
        const SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Polynomial_OverEmpty) {
        const SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo::Zero();
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Polynomial_OverScalar) {
        const SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo::Scalar(0.5);
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, CopyAssign_Polynomial_OverPolynomial) {
        const SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo{SymbolExpression{5, 1.0, false}, SymbolExpression{4, 0.5, true}, SymbolExpression{3, 0.5, true}};
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }


    TEST(Symbolic_SymbolCombo, MoveConstruct_Empty) {
        SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveConstruct_Scalar) {
        SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveConstruct_Polynomial) {
        SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Empty_OverEmpty) {
        SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo::Zero();
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Empty_OverScalar) {
        SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo::Scalar(13.37);
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Empty_OverPolynomial) {
        SymbolCombo src = SymbolCombo::Zero();
        const SymbolCombo ref = SymbolCombo::Zero();

        SymbolCombo test = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Scalar_OverScalar) {
        SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test = SymbolCombo::Scalar(0.2);
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Scalar_OverPolynomial) {
        SymbolCombo src = SymbolCombo::Scalar(0.5);
        const SymbolCombo ref = SymbolCombo::Scalar(0.5);

        SymbolCombo test = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Polynomial_OverEmpty) {
        SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo::Zero();
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)

        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Polynomial_OverScalar) {
        SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo::Scalar(0.5);
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_SymbolCombo, MoveAssign_Polynomial_OverPolynomial) {
        SymbolCombo src = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};
        const SymbolCombo ref = SymbolCombo{SymbolExpression{1, 1.0, false}, SymbolExpression{2, 0.5, true}};

        SymbolCombo test = SymbolCombo{SymbolExpression{5, 1.0, false}, SymbolExpression{4, 0.5, true}, SymbolExpression{3, 0.5, true}};
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)
        
        EXPECT_EQ(test, ref);
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

    TEST(Symbolic_SymbolCombo, IsHermitian) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_Empty) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const SymbolCombo comboEmpty{};
        const auto comboEmptyConj = comboEmpty.conjugate(symbols);
        EXPECT_EQ(comboEmpty, comboEmptyConj);
    }

    TEST(Symbolic_SymbolCombo, Conjugate_Real) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_RealCombo) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_Imaginary) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_RealImaginaryCombo) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_Complex) {
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

    TEST(Symbolic_SymbolCombo, Conjugate_ComplexCombo) {
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


    TEST(Symbolic_SymbolCombo, CastToExpr_Valid) {
        const SymbolCombo combo{SymbolExpression{3, 2.0, false}};

        const SymbolExpression expr{combo};
        EXPECT_EQ(expr, SymbolExpression(3, 2.0, false));
    }

    TEST(Symbolic_SymbolCombo, CastToExpr_Valid2) {
        const SymbolCombo combo{SymbolExpression{5, -2.0, true}};

        const SymbolExpression expr = static_cast<SymbolExpression>(combo);
        EXPECT_EQ(expr, SymbolExpression(5, -2.0, true));
    }

    TEST(Symbolic_SymbolCombo, CastToExpr_Zero) {
        const SymbolCombo zero = SymbolCombo::Zero();

        const SymbolExpression expr = static_cast<SymbolExpression>(zero);
        EXPECT_EQ(expr.id, 0);
    }

    TEST(Symbolic_SymbolCombo, CastToExpr_Bad) {
        const SymbolCombo combo{SymbolExpression{3, 1.0, false}, SymbolExpression{4, 1.0, false}};

        EXPECT_THROW([[maybe_unused]] const SymbolExpression expr = static_cast<SymbolExpression>(combo),
                    std::logic_error);
    }

    TEST(Symbolic_SymbolCombo, AlternativeOrdering) {

        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        SymbolExpression::IdMoreComparator comparator{};

        EXPECT_TRUE(comparator(SymbolExpression{2}, SymbolExpression{1}));
        EXPECT_TRUE(comparator(SymbolExpression{2, false}, SymbolExpression{2, true}));
        EXPECT_FALSE(comparator(SymbolExpression{2, true}, SymbolExpression{2, false}));
        EXPECT_FALSE(comparator(SymbolExpression{1}, SymbolExpression{2}));

        SymbolCombo combo({SymbolExpression{1, 1.0}, SymbolExpression{2, 1.0}, SymbolExpression{5, 2.0, true}},
                          symbols, comparator);

        ASSERT_EQ(combo.size(), 3);
        EXPECT_EQ(combo[0], SymbolExpression(5, 2.0, true));
        EXPECT_EQ(combo[1], SymbolExpression(2, 1.0));
        EXPECT_EQ(combo[2], SymbolExpression(1, 1.0));
        EXPECT_FALSE(combo.is_hermitian(symbols));
        EXPECT_EQ(combo.first_id(), 5);
        EXPECT_EQ(combo.last_id(), 1);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }

    TEST(Symbolic_SymbolCombo, AlternativeOrdering_NontrivialHermitian) {

        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        SymbolExpression::IdMoreComparator comparator{};

        SymbolCombo combo({SymbolExpression{5, 2.0, false}, SymbolExpression{5, 2.0, true}},
                          symbols, comparator);

        ASSERT_EQ(combo.size(), 2);
        EXPECT_EQ(combo[0], SymbolExpression(5, 2.0, false));
        EXPECT_EQ(combo[1], SymbolExpression(5, 2.0, true));
        EXPECT_TRUE(combo.is_hermitian(symbols));
        EXPECT_EQ(combo.first_id(), 5);
        EXPECT_EQ(combo.last_id(), 5);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }

    TEST(Symbolic_SymbolCombo, Append_APlusB) {
        SymbolCombo lhs = SymbolCombo{SymbolExpression{2, 1.0}};
        const SymbolCombo rhs{SymbolExpression{3, 1.0}};

        lhs.append(rhs);
        EXPECT_EQ(lhs, SymbolCombo({SymbolExpression{2, 1.0}, SymbolExpression{3, 1.0}}));
    }

    TEST(Symbolic_SymbolCombo, Append_ZeroPlusA) {
        SymbolCombo lhs = SymbolCombo::Zero();
        const SymbolCombo rhs{SymbolExpression{2, 1.0}};

        lhs.append(rhs);
        EXPECT_EQ(lhs, SymbolCombo(SymbolExpression{2, 1.0}));
    }

    TEST(Symbolic_SymbolCombo, Append_APlusZero) {
        SymbolCombo lhs{SymbolExpression{2, 1.0}};
        const SymbolCombo rhs = SymbolCombo::Zero();

        lhs.append(rhs);
        EXPECT_EQ(lhs, SymbolCombo(SymbolExpression{2, 1.0}));
    }
}