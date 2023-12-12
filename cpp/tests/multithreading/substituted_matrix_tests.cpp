/**
 * substituted_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "../matrix/compare_symbol_matrix.h"
#include "../symbolic/symbolic_matrix_helpers.h"
#include "../symbolic/rules/moment_rule_helpers.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "matrix/monomial_matrix.h"

#include "matrix/polynomial_matrix.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"
#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"


namespace Moment::Tests {
    using namespace Moment::Multithreading;


    class Multithreading_SubstitutedMatrix : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2), 10
            );
            ams_ptr->generate_dictionary(2); // e, a, b, aa, ab (ba), bb
            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ams_ptr->Symbols());
        }

        [[nodiscard]] Algebraic::AlgebraicMatrixSystem& get_system() const noexcept {
            return *this->ams_ptr;
        }

        [[nodiscard]] const Algebraic::AlgebraicContext& get_context() const noexcept {
            return this->ams_ptr->AlgebraicContext();
        };

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ams_ptr->Symbols(); };

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept {
            return this->ams_ptr->polynomial_factory();
        };

        void expect_approximately_equal(const Polynomial& LHS, const Polynomial& RHS) {
            expect_matching_polynomials("Polynomial", LHS, RHS, this->factory_ptr->zero_tolerance);
        }
    };

    TEST_F(Multithreading_SubstitutedMatrix, SubstituteMM_MonoPreserving) {
        auto& system = this->get_system();
        const auto [mm_id, mm] = system.MomentMatrix.create(1);
        ASSERT_EQ(mm_id, 0);
        ASSERT_EQ(mm.Dimension(), 3);

        // Prepare  rulebook <A> -> 0.5
        auto rule_ptr = std::make_unique<MomentRulebook>(this->get_system());
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_rule_polys;
        raw_rule_polys.emplace_back(factory({Monomial(2, 1.0), Monomial(1, -0.5)})); // <a> - 0.5 = 0
        rule_ptr->add_raw_rules(std::move(raw_rule_polys));
        rule_ptr->complete();
        auto [rb_id, rulebook] = system.Rulebook.add(std::move(rule_ptr));
        ASSERT_EQ(rb_id, 0);
        ASSERT_EQ(rulebook.size(), 1);
        ASSERT_TRUE(rulebook.is_monomial());

        // Do MT substitution
        auto [sub_id, sub_mm] = system.SubstitutedMatrix.create(SubstitutedMatrixIndex{mm_id, rb_id},
                                                                MultiThreadPolicy::Always);
        EXPECT_EQ(sub_id, 1);
        EXPECT_EQ(sub_mm.Dimension(), 3);
        EXPECT_TRUE(sub_mm.is_monomial());
    }
}