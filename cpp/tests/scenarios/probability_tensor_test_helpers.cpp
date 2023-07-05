/**
 * probability_tensor_test_helpers.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "probability_tensor_test_helpers.h"

#include "gtest/gtest.h"

namespace Moment::Tests {

    void testIdMmt(const ProbabilityTensorRange& spanID) {

        auto iter = spanID.begin();
        ASSERT_NE(iter, spanID.end());
        EXPECT_TRUE(iter->hasSymbolPoly);
        EXPECT_EQ(iter->symbolPolynomial, Polynomial::Scalar(1.0));
        ++iter;

        EXPECT_EQ(iter, spanID.end());
    }

    void test2Mmt(const ProbabilityTensorRange &spanA,
                  symbol_name_t id, symbol_name_t alice,
                  const std::string &ctx) {

        auto iter = spanA.begin();
        ASSERT_NE(iter, spanA.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{alice, 1.0})) << ctx;
        ++iter;

        ASSERT_NE(iter, spanA.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{id, 1.0}, Monomial{alice, -1.0}})) << ctx;
        ++iter;

        EXPECT_EQ(iter, spanA.end()) << ctx;
    }

    void testSingleCV(const ProbabilityTensorRange &span,
                      symbol_name_t id, const std::string &ctx) {
        auto iter = span.begin();
        ASSERT_NE(iter, span.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{id, 1.0})) << ctx;
        ++iter;

        EXPECT_EQ(iter, span.end()) << ctx;
    }


    void test3Mmt(const ProbabilityTensorRange &spanA,
              symbol_name_t id, symbol_name_t a0, symbol_name_t a1,
              const std::string &ctx) {

        auto iter = spanA.begin();
        ASSERT_NE(iter, spanA.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{a0, 1.0})) << ctx;
        ++iter;

        ASSERT_NE(iter, spanA.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{a1, 1.0})) << ctx;
        ++iter;

        ASSERT_NE(iter, spanA.end()) << ctx;
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial,
                  Polynomial({Monomial{id, 1.0}, Monomial{a0, -1.0}, Monomial{a1, -1.0}})) << ctx;
        ++iter;

        EXPECT_EQ(iter, spanA.end()) << ctx;
    }

    void test22JoinMmt(const ProbabilityTensorRange &spanAB,
                       symbol_name_t id,
                       symbol_name_t alice,
                       symbol_name_t bob,
                       symbol_name_t alice_bob,
                       const std::string& ctx) {

        auto iter = spanAB.begin();
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{alice_bob, 1.0})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{bob, 1.0}, Monomial{alice_bob, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{alice, 1.0}, Monomial{alice_bob, -1.0}})) << ctx;


        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        if (alice == bob) {
            EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{id, 1.0},
                                                          Monomial{alice, -2.0},
                                                          Monomial{alice_bob, 1.0}})) << ctx;
        } else {
            EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{id, 1.0},
                                                          Monomial{alice, -1.0},
                                                          Monomial{bob, -1.0},
                                                          Monomial{alice_bob, 1.0}})) << ctx;
        }

        ++iter;
        EXPECT_EQ(iter, spanAB.end());
    }

    void test32JoinMmt(const ProbabilityTensorRange& spanAB,
                       symbol_name_t id,
                       symbol_name_t a0,
                       symbol_name_t a1,
                       symbol_name_t b,
                       symbol_name_t a0b,
                       symbol_name_t a1b,
                       const std::string& ctx) {

        auto iter = spanAB.begin();
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{a0b, 1.0})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{a1b, 1.0})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{b, 1.0}, Monomial{a0b, -1.0}, Monomial{a1b, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{a0, 1.0}, Monomial{a0b, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{a1, 1.0}, Monomial{a1b, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanAB.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{id, 1.0},
                                                      Monomial{a0, -1.0}, Monomial{a1, -1.0}, Monomial{b, -1.0},
                                                      Monomial{a0b, 1.0}, Monomial{a1b, 1.0}})) << ctx;


        ++iter;
        EXPECT_EQ(iter, spanAB.end());
    }

    void test222JoinMmt( const ProbabilityTensorRange& spanABC,
                        symbol_name_t id,
                        symbol_name_t alice,
                        symbol_name_t bob,
                        symbol_name_t charlie,
                        symbol_name_t alice_bob,
                        symbol_name_t alice_charlie,
                        symbol_name_t bob_charlie,
                        symbol_name_t alice_bob_charlie,
                        const std::string& ctx) {

        auto iter = spanABC.begin();
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial(Monomial{alice_bob_charlie, 1.0})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{bob_charlie, 1.0},
                                                      Monomial{alice_bob_charlie, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{alice_charlie, 1.0},
                                                      Monomial{alice_bob_charlie, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{charlie, 1.0},
                                                      Monomial{alice_charlie, -1.0},
                                                      Monomial{bob_charlie, -1.0},
                                                      Monomial{alice_bob_charlie, 1.0}})) << ctx;
        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{alice_bob, 1.0},
                                                      Monomial{alice_bob_charlie, -1.0}})) << ctx;

        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{bob, 1.0},
                                                      Monomial{alice_bob, -1.0},
                                                      Monomial{bob_charlie, -1.0},
                                                      Monomial{alice_bob_charlie, 1.0}})) << ctx;
        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{alice, 1.0},
                                                      Monomial{alice_bob, -1.0},
                                                      Monomial{alice_charlie, -1.0},
                                                      Monomial{alice_bob_charlie, 1.0}})) << ctx;
        ++iter;
        ASSERT_NE(iter, spanABC.end());
        EXPECT_TRUE(iter->hasSymbolPoly) << ctx;
        EXPECT_EQ(iter->symbolPolynomial, Polynomial({Monomial{id, 1.0},
                                                      Monomial{alice, -1.0},
                                                      Monomial{bob, -1.0},
                                                      Monomial{charlie, -1.0},
                                                      Monomial{alice_bob, 1.0},
                                                      Monomial{alice_charlie, 1.0},
                                                      Monomial{bob_charlie, 1.0},
                                                      Monomial{alice_bob_charlie, -1.0}})) << ctx;

        ++iter;
        EXPECT_EQ(iter, spanABC.end());

    }
}