/**
 * polynomial_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/imported/imported_matrix_system.h"


namespace Moment::Tests {
    TEST(Symbolic_Polynomial, Create_Empty) {
        Polynomial empty{};
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.begin(), empty.end());
        EXPECT_TRUE(empty.is_monomial());
        EXPECT_TRUE(empty.real_factors());
        EXPECT_EQ(empty.first_id(), 0);
        EXPECT_EQ(empty.last_id(), 0);
    }

    TEST(Symbolic_Polynomial, Create_Scalar) {
        Polynomial scalar = Polynomial::Scalar(2.5);
        EXPECT_FALSE(scalar.empty());
        ASSERT_EQ(scalar.size(), 1);
        EXPECT_TRUE(scalar.is_monomial());
        EXPECT_TRUE(scalar.real_factors());
        EXPECT_EQ(*scalar.begin(), Monomial(1, 2.5));

        EXPECT_EQ(scalar.first_id(), 1);
        EXPECT_EQ(scalar.last_id(), 1);

    }
    TEST(Symbolic_Polynomial, Create_ComplexScalar) {
        Polynomial scalar = Polynomial::Scalar({2.5, 1.0});
        EXPECT_FALSE(scalar.empty());
        ASSERT_EQ(scalar.size(), 1);
        EXPECT_TRUE(scalar.is_monomial());
        EXPECT_FALSE(scalar.real_factors());
        EXPECT_EQ(*scalar.begin(), Monomial(1, {2.5, 1.0}));

        EXPECT_EQ(scalar.first_id(), 1);
        EXPECT_EQ(scalar.last_id(), 1);

    }

    TEST(Symbolic_Polynomial, Create_OneElem) {
        Polynomial one_elem{Monomial{13, -2.0}};
        EXPECT_FALSE(one_elem.empty());
        ASSERT_EQ(one_elem.size(), 1);
        EXPECT_TRUE(one_elem.is_monomial());
        EXPECT_TRUE(one_elem.real_factors());
        EXPECT_EQ(*one_elem.begin(), Monomial(13, -2.0));

        EXPECT_EQ(one_elem.first_id(), 13);
        EXPECT_EQ(one_elem.last_id(), 13);
    }

    TEST(Symbolic_Polynomial, Create_ThreeElems) {
        Polynomial threeElems{Monomial{2, 13.0}, Monomial{10, 100.0}, Monomial{5, -23.0}};
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

    TEST(Symbolic_Polynomial, Create_InitListZero) {
        Polynomial empty{Monomial{0, 1.0}};
        EXPECT_TRUE(empty.empty()) << empty;
        EXPECT_EQ(empty.size(), 0)  << empty;
        EXPECT_EQ(empty.begin(), empty.end())  << empty;
        EXPECT_TRUE(empty.is_monomial()) << empty;
    }


    TEST(Symbolic_Polynomial, Create_Overlapped1) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{2, 30.0},
                                Monomial{2, 20.0}, Monomial{3, 40.0}};
        const Polynomial expected{Monomial{1, 10.0}, Monomial{2, 50.0}, Monomial{3, 40.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_Overlapped2) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{2, 30.0},
                                Monomial{1, 20.0}, Monomial{2, 40.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{2, 70.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_Overlapped3) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{2, 30.0}, Monomial{3, 50.0},
                                Monomial{1, 20.0}, Monomial{2, 40.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{2, 70.0}, Monomial{3, 50.0}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_OverlappedToZero) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{1, -10.0}};
        const Polynomial expected = Polynomial();
        EXPECT_EQ(actual, expected);
        EXPECT_TRUE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_OverlappedWithZero1) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{1, -10.0}, Monomial{2, 20.0}};
        const Polynomial expected = Polynomial{{Monomial{2, 20.0}}};
        EXPECT_EQ(actual, expected);
        EXPECT_TRUE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_OverlappedWithZero2) {
        const Polynomial actual{Monomial{1, 10.0}, Monomial{2, -20.0},
                                Monomial{2, 20.0}, Monomial{3, 10.0}};
        const Polynomial expected = Polynomial{{Monomial{1, 10.0}, Monomial{3, 10.0}}};
        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(expected.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_FromMap) {
        std::map<symbol_name_t, double> testMap{{2,  13.0},
                                                {10, 100.0},
                                                {5,  -23.0}};

        Polynomial threeElems{testMap};
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


    TEST(Symbolic_Polynomial, Create_FromExpr) {
        const Monomial expr{5, -2.0, true};
        const Polynomial combo{expr};
        ASSERT_EQ(combo.size(), 1);
        EXPECT_EQ(*combo.begin(), Monomial(5, -2.0, true));
        EXPECT_TRUE(combo.is_monomial());
    }

    TEST(Symbolic_Polynomial, Create_FromExprZero) {
        const Monomial expr{0, 1.0};
        const Polynomial combo(expr); // <- can't use {} as this will call init_list c'tor!!
        ASSERT_EQ(combo.size(), 0);
        EXPECT_TRUE(combo.is_monomial());
    }


    TEST(Symbolic_Polynomial, CopyConstruct_Empty) {
        const Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyConstruct_Scalar) {
        const Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyConstruct_Polynomial) {
        const Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test{src}; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Empty_OverEmpty) {
        const Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial();
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Empty_OverScalar) {
        const Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial::Scalar(13.37);
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Empty_OverPolynomial) {
        const Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Scalar_OverScalar) {
        const Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test = Polynomial::Scalar(0.2);
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Scalar_OverPolynomial) {
        const Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        test = src;
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Polynomial_OverEmpty) {
        const Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial();
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Polynomial_OverScalar) {
        const Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial::Scalar(0.5);
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, CopyAssign_Polynomial_OverPolynomial) {
        const Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial{Monomial{5, 1.0, false}, Monomial{4, 0.5, true}, Monomial{3, 0.5, true}};
        test = src; // NOLINT(performance-unnecessary-copy-initialization)
        EXPECT_EQ(src, ref);
        EXPECT_EQ(test, ref);
    }


    TEST(Symbolic_Polynomial, MoveConstruct_Empty) {
        Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveConstruct_Scalar) {
        Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveConstruct_Polynomial) {
        Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test{std::move(src)};
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Empty_OverEmpty) {
        Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial();
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Empty_OverScalar) {
        Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial::Scalar(13.37);
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Empty_OverPolynomial) {
        Polynomial src = Polynomial();
        const Polynomial ref = Polynomial();

        Polynomial test = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Scalar_OverScalar) {
        Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test = Polynomial::Scalar(0.2);
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Scalar_OverPolynomial) {
        Polynomial src = Polynomial::Scalar(0.5);
        const Polynomial ref = Polynomial::Scalar(0.5);

        Polynomial test = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        test = std::move(src);
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Polynomial_OverEmpty) {
        Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial();
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)

        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Polynomial_OverScalar) {
        Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial::Scalar(0.5);
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)
        
        EXPECT_EQ(test, ref);
    }

    TEST(Symbolic_Polynomial, MoveAssign_Polynomial_OverPolynomial) {
        Polynomial src = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};
        const Polynomial ref = Polynomial{Monomial{1, 1.0, false}, Monomial{2, 0.5, true}};

        Polynomial test = Polynomial{Monomial{5, 1.0, false}, Monomial{4, 0.5, true}, Monomial{3, 0.5, true}};
        test = std::move(src); // NOLINT(performance-unnecessary-copy-initialization)
        
        EXPECT_EQ(test, ref);
    }


    TEST(Symbolic_Polynomial, Equality) {
        Polynomial listA{Monomial{2, 10.0}, Monomial{5, 20.0}};
        Polynomial listB{Monomial{2, 10.0}, Monomial{5, 20.0}};
        Polynomial listC{Monomial{2, 10.0}, Monomial{10, 20.0}};
        Polynomial listD{Monomial{2, 10.0}, Monomial{10, 19.0}};
        Polynomial listE{Monomial{2, 10.0}};
        Polynomial listF{Monomial{2, 10.0}, Monomial{5, 40.0}};

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

    TEST(Symbolic_Polynomial, Addition_NoOverlap) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{2, 20.0}};
        const Polynomial listB{Monomial{3, 30.0}, Monomial{4, 40.0}};
        const Polynomial expected{Monomial{1, 10.0}, Monomial{2, 20.0},
                                  Monomial{3, 30.0}, Monomial{4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_Polynomial, Addition_Interleaved) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{3, 30.0}};
        const Polynomial listB{Monomial{2, 20.0}, Monomial{4, 40.0}};
        const Polynomial expected{Monomial{1, 10.0}, Monomial{2, 20.0},
                                  Monomial{3, 30.0}, Monomial{4, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_Polynomial, Addition_Overlapped1) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{2, 30.0}};
        const Polynomial listB{Monomial{2, 20.0}, Monomial{3, 40.0}};
        const Polynomial expected{Monomial{1, 10.0}, Monomial{2, 50.0}, Monomial{3, 40.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_Polynomial, Addition_Overlapped2) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{2, 30.0}};
        const Polynomial listB{Monomial{1, 20.0}, Monomial{2, 40.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{2, 70.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_Polynomial, Addition_Overlapped3) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{2, 30.0}, Monomial{3, 50.0}};
        const Polynomial listB{Monomial{1, 20.0}, Monomial{2, 40.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{2, 70.0}, Monomial{3, 50.0}};
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }

    TEST(Symbolic_Polynomial, Addition_ToZero) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{2, 30.0}};
        const Polynomial listB{Monomial{1, -10.0}, Monomial{2, -30.0}};
        const Polynomial expected = Polynomial();
        auto actualAB = listA + listB;
        EXPECT_EQ(actualAB, expected);
        auto actualBA = listB + listA;
        EXPECT_EQ(actualBA, expected);
    }


    TEST(Symbolic_Polynomial, SelfAddition) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{3, 30.0}};
        const Polynomial listB{Monomial{2, 20.0}, Monomial{4, 40.0}};
        const Polynomial expected{Monomial{1, 10.0}, Monomial{2, 20.0},
                                  Monomial{3, 30.0}, Monomial{4, 40.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list += listB;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_Polynomial, PostMultiply) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{3, 30.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{3, 90.0}};

        auto list = listA;
        EXPECT_EQ(list, listA);
        list *= 3;
        EXPECT_NE(list, listA);
        EXPECT_EQ(list, expected);
    }

    TEST(Symbolic_Polynomial, MultiplyFactor) {
        const Polynomial listA{Monomial{1, 10.0}, Monomial{3, 30.0}};
        const Polynomial expected{Monomial{1, 30.0}, Monomial{3, 90.0}};

        const auto listB = listA * 3;
        EXPECT_NE(listA, listB);
        EXPECT_EQ(listB, expected);
    }

    TEST(Symbolic_Polynomial, IsHermitian) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial comboEmpty{};
        EXPECT_TRUE(comboEmpty.is_hermitian(symbols, 1.0));

        const Polynomial combo_H_Id{Monomial{1, 1.0}};
        EXPECT_TRUE(combo_H_Id.is_hermitian(symbols, 1.0));

        const Polynomial combo_H_A{Monomial{2, 1.0}};
        EXPECT_TRUE(combo_H_A.is_hermitian(symbols, 1.0));

        const Polynomial combo_iA{Monomial{2, std::complex{0.0, 1.0}}};
        EXPECT_FALSE(combo_iA.is_hermitian(symbols, 1.0));

        const Polynomial combo_H_B_Bstar{Monomial{3, 1.0}, Monomial{3, 1.0, true}};
        EXPECT_TRUE(combo_H_B_Bstar.is_hermitian(symbols, 1.0));

        Polynomial combo_H_C_Cstar{Monomial{4, 1.0}, Monomial{4, 1.0, true}}; // Ill-formed, should be zero.
        combo_H_C_Cstar.fix_cc_in_place(symbols, true, 1.0);
        EXPECT_TRUE(combo_H_C_Cstar.is_hermitian(symbols, 1.0));

        const Polynomial combo_iD{Monomial{4, std::complex{0.0, 1.0}}};
        EXPECT_TRUE(combo_iD.is_hermitian(symbols, 1.0));

        const Polynomial combo_Id_B{Monomial{1, 1.0}, Monomial{3, 1.0}};
        EXPECT_FALSE(combo_Id_B.is_hermitian(symbols, 1.0));

        const Polynomial combo_B{Monomial{3, 1.0}};
        EXPECT_FALSE(combo_B.is_hermitian(symbols, 1.0));

        const Polynomial combo_B_3Bstar{Monomial{3, 1.0}, Monomial{3, 2.0, true}};
        EXPECT_FALSE(combo_B_3Bstar.is_hermitian(symbols, 1.0));

        const Polynomial combo_complex_H{Monomial{3, std::complex{0.0, -1.0}, false},
                                         Monomial{3, std::complex{0.0, 1.0}, true}}; // -iX + iX*
        EXPECT_TRUE(combo_complex_H.is_hermitian(symbols, 1.0));
    }

    TEST(Symbolic_Polynomial, Conjugate_Empty) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial comboEmpty{};
        const auto comboEmptyConj = comboEmpty.conjugate(symbols);
        EXPECT_EQ(comboEmpty, comboEmptyConj);
    }

    TEST(Symbolic_Polynomial, Conjugate_Real) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{2, 2.0, false}};
        const Polynomial comboConjExp{Monomial{2, 2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_Polynomial, Conjugate_RealCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{1, 1.0, false}, Monomial{2, 2.0, false}};
        const Polynomial comboConjExp{Monomial{1, 1.0, false}, Monomial{2, 2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_Polynomial, Conjugate_Imaginary) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{4, 2.0, false}};
        const Polynomial comboConjExp{Monomial{4, -2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_Polynomial, Conjugate_RealImaginaryCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{1, 1.0, false}, Monomial{4, 2.0, false}};
        const Polynomial comboConjExp{Monomial{1, 1.0, false}, Monomial{4, -2.0, false}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_Polynomial, Conjugate_Complex) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{3, 2.0, false}};
        const Polynomial comboConjExp{Monomial{3, 2.0, true}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }

    TEST(Symbolic_Polynomial, Conjugate_ComplexCombo) {
        Imported::ImportedMatrixSystem ims;
        auto &symbols = ims.Symbols();
        symbols.create(true, false); // 2 real
        symbols.create(true, true); // 3 complex
        symbols.create(false, true); // 4 imaginary

        const Polynomial combo{Monomial{3, 2.0, false}, Monomial{3, 1.0, true}};
        const Polynomial comboConjExp{Monomial{3, 1.0, false}, Monomial{3, 2.0, true}};
        const auto comboConj = combo.conjugate(symbols);
        EXPECT_EQ(comboConj, comboConjExp);
    }


    TEST(Symbolic_Polynomial, CastToExpr_Valid) {
        const Polynomial combo{Monomial{3, 2.0, false}};

        const Monomial expr{combo};
        EXPECT_EQ(expr, Monomial(3, 2.0, false));
    }

    TEST(Symbolic_Polynomial, CastToExpr_Valid2) {
        const Polynomial combo{Monomial{5, -2.0, true}};

        const Monomial expr = static_cast<Monomial>(combo);
        EXPECT_EQ(expr, Monomial(5, -2.0, true));
    }

    TEST(Symbolic_Polynomial, CastToExpr_Zero) {
        const Polynomial zero = Polynomial();

        const Monomial expr = static_cast<Monomial>(zero);
        EXPECT_EQ(expr.id, 0);
    }

    TEST(Symbolic_Polynomial, CastToExpr_Bad) {
        const Polynomial combo{Monomial{3, 1.0, false}, Monomial{4, 1.0, false}};

        EXPECT_THROW([[maybe_unused]] const Monomial expr = static_cast<Monomial>(combo),
                    std::logic_error);
    }

    TEST(Symbolic_Polynomial, AlternativeOrdering) {

        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        Monomial::IdMoreComparator comparator{};

        EXPECT_TRUE(comparator(Monomial{2}, Monomial{1}));
        EXPECT_TRUE(comparator(Monomial{2, false}, Monomial{2, true}));
        EXPECT_FALSE(comparator(Monomial{2, true}, Monomial{2, false}));
        EXPECT_FALSE(comparator(Monomial{1}, Monomial{2}));

        Polynomial combo({Monomial{1, 1.0}, Monomial{2, 1.0}, Monomial{5, 2.0, true}},
                         symbols, comparator);

        ASSERT_EQ(combo.size(), 3);
        EXPECT_EQ(combo[0], Monomial(5, 2.0, true));
        EXPECT_EQ(combo[1], Monomial(2, 1.0));
        EXPECT_EQ(combo[2], Monomial(1, 1.0));
        EXPECT_FALSE(combo.is_hermitian(symbols, 1.0));
        EXPECT_EQ(combo.first_id(), 5);
        EXPECT_EQ(combo.last_id(), 1);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }

    TEST(Symbolic_Polynomial, AlternativeOrdering_NontrivialHermitian) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        Monomial::IdMoreComparator comparator{};

        Polynomial combo({Monomial{5, 2.0, false}, Monomial{5, 2.0, true}},
                         symbols, comparator);

        ASSERT_EQ(combo.size(), 2);
        EXPECT_EQ(combo[0], Monomial(5, 2.0, false));
        EXPECT_EQ(combo[1], Monomial(5, 2.0, true));
        EXPECT_TRUE(combo.is_hermitian(symbols, 1.0));
        EXPECT_EQ(combo.first_id(), 5);
        EXPECT_EQ(combo.last_id(), 5);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }

    TEST(Symbolic_Polynomial, Append_APlusB) {
        Polynomial lhs = Polynomial{Monomial{2, 1.0}};
        const Polynomial rhs{Monomial{3, 1.0}};

        lhs.append(rhs);
        EXPECT_EQ(lhs, Polynomial({Monomial{2, 1.0}, Monomial{3, 1.0}}));
    }

    TEST(Symbolic_Polynomial, Append_ZeroPlusA) {
        Polynomial lhs = Polynomial();
        const Polynomial rhs{Monomial{2, 1.0}};

        lhs.append(rhs);
        EXPECT_EQ(lhs, Polynomial(Monomial{2, 1.0}));
    }

    TEST(Symbolic_Polynomial, Append_APlusZero) {
        Polynomial lhs{Monomial{2, 1.0}};
        const Polynomial rhs = Polynomial();

        lhs.append(rhs);
        EXPECT_EQ(lhs, Polynomial(Monomial{2, 1.0}));
    }

}