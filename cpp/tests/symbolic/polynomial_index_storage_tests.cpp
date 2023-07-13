/**
 * polynomial_to_index_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix_system/polynomial_localizing_matrix_index.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "matrix_system/polynomial_index_storage.h"
#include "symbolic/symbol_table.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"


namespace Moment::Tests {

    class Symbolic_PolynomialToIndex : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ms_ptr;

    protected:
        void SetUp() override {
            // One party, two symbols
            this->ms_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2)
                            );

            this->ms_ptr->generate_dictionary(2); //

            const auto &symbols = this->ms_ptr->Symbols();
            ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
            ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
            ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);
            ASSERT_FALSE(symbols[5].is_hermitian());
        }

        [[nodiscard]] SymbolTable &get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory &get_factory() noexcept { return this->ms_ptr->polynomial_factory(); }
    };


    TEST_F(Symbolic_PolynomialToIndex, Empty) {
        PolynomialIndexStorage index{this->get_factory()};

        EXPECT_TRUE(index.empty());
        EXPECT_EQ(index.size(), 0);
    }

    TEST_F(Symbolic_PolynomialToIndex, AddThenFind_One) {
        const auto& factory = this->get_factory();
        PolynomialIndexStorage index{factory};
        const PolynomialLMIndex key = {1, factory({Monomial{1, 1.0}})};


        index.insert(key, 13);
        EXPECT_FALSE(index.empty());
        EXPECT_EQ(index.size(), 1);

        auto where = index.find(key);
        EXPECT_EQ(where, 13);

        EXPECT_EQ(index.find({2, factory({Monomial{1, 1.0}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{1, 0.9}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}, Monomial{1, 1.0}})}), -1);

    }

    TEST_F(Symbolic_PolynomialToIndex, AddThenFind_SeparateHeads) {
        const auto& factory = this->get_factory();
        PolynomialIndexStorage index{factory};
        const PolynomialLMIndex keyA = {1, factory({Monomial{1, 1.0}})};
        const PolynomialLMIndex keyB = {1, factory({Monomial{2, -2.0}})};

        index.insert(keyA, 13);
        index.insert(keyB, 17);

        EXPECT_FALSE(index.empty());
        EXPECT_EQ(index.size(), 2);

        const auto whereA = index.find(keyA);
        EXPECT_EQ(whereA, 13);

        const auto whereB = index.find(keyB);
        EXPECT_EQ(whereB, 17);


        EXPECT_EQ(index.find({2, factory({Monomial{1, 1.0}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{1, 0.9}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}, Monomial{1, 1.0}})}), -1);
    }

    TEST_F(Symbolic_PolynomialToIndex, AddThenFind_Similar) {
        const auto& factory = this->get_factory();
        PolynomialIndexStorage index{factory};
        const PolynomialLMIndex keyA = {1, factory({Monomial{1, 1.0}})};
        const PolynomialLMIndex keyB = {1, factory({Monomial{1, 1.1}})};

        index.insert(keyA, 13);
        index.insert(keyB, 17);

        EXPECT_FALSE(index.empty());
        EXPECT_EQ(index.size(), 2);

        const auto whereA = index.find(keyA);
        EXPECT_EQ(whereA, 13);

        const auto whereB = index.find(keyB);
        EXPECT_EQ(whereB, 17);

        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{1, 0.9}})}), -1);
        EXPECT_EQ(index.find({1, factory({Monomial{2, 1.0}, Monomial{1, 1.0}})}), -1);
    }

    TEST_F(Symbolic_PolynomialToIndex, FindOrInsert) {
        const auto& factory = this->get_factory();

        PolynomialIndexStorage index{factory};
        const PolynomialLMIndex keyA = {1, factory({Monomial{2, 1.0}})};
        const PolynomialLMIndex keyB = {1, factory({Monomial{1, 1.1}})};

        auto [q1_index, q1_insert] = index.insert(keyA, 13);
        EXPECT_EQ(q1_index, 13);
        EXPECT_EQ(q1_insert, true);
        EXPECT_EQ(index.size(), 1);

        auto [q2_index, q2_insert] = index.insert(keyA, 15);
        EXPECT_EQ(q2_index, 13);
        EXPECT_EQ(q2_insert, false);
        EXPECT_EQ(index.size(), 1);

        auto [q3_index, q3_insert] = index.insert(keyB, 18);
        EXPECT_EQ(q3_index, 18);
        EXPECT_EQ(q3_insert, true);
        EXPECT_EQ(index.size(), 2);

    }

}
