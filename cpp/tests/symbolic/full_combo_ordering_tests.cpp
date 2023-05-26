/**
 * full_combo_ordering_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "symbolic/full_combo_ordering.h"

namespace Moment::Tests {

    class Symbolic_FullComboOrdering : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;
        std::unique_ptr<SymbolComboFactory> factory_ptr;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                        std::make_unique<Algebraic::AlgebraicContext>(2)
            );
            ams_ptr->generate_dictionary(2);
            factory_ptr = std::make_unique<SymbolComboFactory>(ams_ptr->Symbols());
        }

        [[nodiscard]] const Algebraic::AlgebraicContext& get_context() const noexcept {
            return this->ams_ptr->AlgebraicContext();
        };

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ams_ptr->Symbols(); };

        [[nodiscard]] const SymbolComboFactory& get_factory() const noexcept { return *this->factory_ptr; };

    };

    TEST_F(Symbolic_FullComboOrdering, BothZero) {
        FullComboOrdering fco{this->get_factory()};
        EXPECT_FALSE(fco(Polynomial(), Polynomial()));
    }

    TEST_F(Symbolic_FullComboOrdering, BothScalar) {
        FullComboOrdering fco{this->get_factory()};
        EXPECT_FALSE(fco(Polynomial::Scalar(1.0), Polynomial::Scalar(2.0)));
        EXPECT_FALSE(fco(Polynomial::Scalar(2.0), Polynomial::Scalar(1.0)));
    }

    TEST_F(Symbolic_FullComboOrdering, ThreeVsTwo) {
        const auto& f = this->get_factory();
        FullComboOrdering fco{f};
        EXPECT_FALSE(fco(f({Monomial{3, 1.0}}),
                         f({Monomial{2, 1.0}})));
        EXPECT_TRUE(fco(f({Monomial{2, 1.0}}),
                        f({Monomial{3, 1.0}})));
    }

    TEST_F(Symbolic_FullComboOrdering, ThreeVsTwoPlusOne) {
        const auto& f = this->get_factory();
        FullComboOrdering fco{f};
        EXPECT_FALSE(fco(f({Monomial{3, 1.0}}),
                         f({Monomial{2, 1.0}, Monomial{1, 1.0}})));
        EXPECT_TRUE(fco(f({Monomial{2, 1.0}, Monomial{1, 1.0}}),
                        f({Monomial{3, 1.0}})));
    }

    TEST_F(Symbolic_FullComboOrdering, ThreeVsThreePlusTwo) {
        const auto& f = this->get_factory();
        FullComboOrdering fco{f};
        EXPECT_FALSE(fco(f({Monomial{3, 1.0}, Monomial{2, 1.0}}),
                        f({Monomial{3, 1.0}})));
        EXPECT_TRUE(fco(f({Monomial{3, 1.0}}),
                        f({Monomial{3, 1.0}, Monomial{2, 1.0}}))); // "3 < 3 + 2"
    }
}