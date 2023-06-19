/**
 * polynomial_to_basis_mask_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"


#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/polynomial_to_basis_mask.h"
#include "symbolic/symbol_table.h"

#include "matrix_system.h"


#include <memory>

namespace Moment::Tests {


    class Symbolic_PolynomialToBasisMask : public ::testing::Test {
    private:
        std::unique_ptr<MatrixSystem> ms_ptr;

    protected:
        void SetUp() override {
            // One party, two symbols
            this->ms_ptr = std::make_unique<MatrixSystem>(std::make_unique<Context>(3));

            this->ms_ptr->generate_dictionary(2); // 0 1 a b c aa ab ac bb bc  cc
            const auto &symbols = this->ms_ptr->Symbols();
            ASSERT_EQ(symbols.size(), 11);
            ASSERT_EQ(symbols.Basis.RealSymbolCount(), 10);
            ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 3); // 6 = ab, 7 = ac, 9 = bc
            ASSERT_FALSE(symbols[6].is_hermitian());
            ASSERT_FALSE(symbols[7].is_hermitian());
            ASSERT_FALSE(symbols[9].is_hermitian());
        }

        [[nodiscard]] SymbolTable &get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory& get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }
    };



    TEST_F(Symbolic_PolynomialToBasisMask, EmptyMask) {
        const auto& symbols = this->get_symbols();
        PolynomialToBasisMask ptbm{symbols, this->get_factory().zero_tolerance};

        auto [re_mask, im_mask] = ptbm.empty_mask();
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 0);

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 0);

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 0);
        EXPECT_EQ(im_set.size(), 0);
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Zero) {
        const auto& symbols = this->get_symbols();
        PolynomialToBasisMask ptbm{symbols, this->get_factory().zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Polynomial zero = Polynomial::Zero();
        ptbm.set_bits(re_mask, im_mask, zero);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 0);

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 0);

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 0);
        EXPECT_EQ(im_set.size(), 0);
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Monomial) {
        const auto& symbols = this->get_symbols();
        PolynomialToBasisMask ptbm{symbols, this->get_factory().zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Monomial ac = Monomial{7};
        ptbm.set_bits(re_mask, im_mask, ac);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 1);
        EXPECT_TRUE(re_mask.test(6));

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 1);
        EXPECT_TRUE(im_mask.test(1));

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 1);
        EXPECT_TRUE(re_set.contains(6));
        EXPECT_EQ(im_set.size(), 1);
        EXPECT_TRUE(im_set.contains(1));
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Polynomial_Simple) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        PolynomialToBasisMask ptbm{symbols, factory.zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Polynomial poly{factory({Monomial{3}, Monomial{7,-2.0, true}})}; // b - 2ac*
        ptbm.set_bits(re_mask, im_mask, poly);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 2);
        EXPECT_TRUE(re_mask.test(2));
        EXPECT_TRUE(re_mask.test(6));

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 1);
        EXPECT_TRUE(im_mask.test(1));

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 2);
        EXPECT_TRUE(re_set.contains(2));
        EXPECT_TRUE(re_set.contains(6));
        EXPECT_EQ(im_set.size(), 1);
        EXPECT_TRUE(im_set.contains(1));
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Polynomial_NoCancel) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        PolynomialToBasisMask ptbm{symbols, factory.zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Polynomial poly{factory({Monomial{3}, Monomial{7, 1.0, false}, Monomial{7,-2.0, true}})}; // b + ac - 2ac*
        ptbm.set_bits(re_mask, im_mask, poly);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 2);
        EXPECT_TRUE(re_mask.test(2));
        EXPECT_TRUE(re_mask.test(6));

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 1);
        EXPECT_TRUE(im_mask.test(1));

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 2);
        EXPECT_TRUE(re_set.contains(2));
        EXPECT_TRUE(re_set.contains(6));
        EXPECT_EQ(im_set.size(), 1);
        EXPECT_TRUE(im_set.contains(1));
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Polynomial_CancelImaginaryPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        PolynomialToBasisMask ptbm{symbols, factory.zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Polynomial poly{factory({Monomial{3}, Monomial{7, 1.0, false}, Monomial{7, 1.0, true}})}; // b + ac + ac*
        ptbm.set_bits(re_mask, im_mask, poly);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 2);
        EXPECT_TRUE(re_mask.test(2));
        EXPECT_TRUE(re_mask.test(6));

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 0);

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 2);
        EXPECT_TRUE(re_set.contains(2));
        EXPECT_TRUE(re_set.contains(6));
        EXPECT_EQ(im_set.size(), 0);
    }

    TEST_F(Symbolic_PolynomialToBasisMask, Polynomial_CancelRealPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        PolynomialToBasisMask ptbm{symbols, factory.zero_tolerance};
        auto [re_mask, im_mask] = ptbm.empty_mask();

        const Polynomial poly{factory({Monomial{3}, Monomial{7, 1.0, false}, Monomial{7, -1.0, true}})}; // b + ac - ac*
        ptbm.set_bits(re_mask, im_mask, poly);
        EXPECT_EQ(re_mask.bit_size, symbols.Basis.RealSymbolCount());
        EXPECT_EQ(re_mask.count(), 1);
        EXPECT_TRUE(re_mask.test(2));

        EXPECT_EQ(im_mask.bit_size, symbols.Basis.ImaginarySymbolCount());
        EXPECT_EQ(im_mask.count(), 1);
        EXPECT_TRUE(im_mask.test(1));

        auto [re_set, im_set] = Moment::PolynomialToBasisMask::masks_to_sets(re_mask, im_mask);
        EXPECT_EQ(re_set.size(), 1);
        EXPECT_TRUE(re_set.contains(2));
        EXPECT_EQ(im_set.size(), 1);
        EXPECT_TRUE(im_set.contains(1));
    }

}

