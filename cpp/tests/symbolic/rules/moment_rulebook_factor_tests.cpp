/**
 * moment_rulebook_factor_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"

#include "../symbolic_matrix_helpers.h"

namespace Moment::Tests {

    class Symbolic_MomentRulebook_Factor : public ::testing::Test {
    private:
        std::unique_ptr<Inflation::InflationMatrixSystem> ims_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        symbol_name_t id_a, id_b, id_c, id_ab, id_ac, id_bc, id_abc;


        symbol_name_t id_aa, id_bb, id_cc, id_aaa, id_abb, id_acc, id_bbb, id_aab, id_bcc;


    protected:
        void SetUp() override {
            using namespace Inflation;
            this->ims_ptr = std::make_unique<InflationMatrixSystem>(
                    std::make_unique<InflationContext>(CausalNetwork{{0, 0, 0}, {}}, 1));

            ims_ptr->generate_dictionary(3); // Factors up to <A><B><C>
            const auto& factors = this->get_factors();

            ASSERT_FALSE(factors.empty());
            ASSERT_EQ(factors.size(), ims_ptr->Symbols().size());
            this->id_a = factors.find_index_by_factors({2}).value();
            this->id_b = factors.find_index_by_factors({3}).value();
            this->id_c = factors.find_index_by_factors({4}).value();
            this->id_ab = factors.find_index_by_factors({2, 3}).value();
            this->id_ac = factors.find_index_by_factors({2, 4}).value();
            this->id_bc = factors.find_index_by_factors({3, 4}).value();
            this->id_abc = factors.find_index_by_factors({2, 3, 4}).value();

            // Find not by factor
            const auto& symbols = this->get_symbols();

            this->id_aa = find_or_fail(this->get_symbols(), OperatorSequence{{0, 0}, this->get_context()});
            this->id_bb = find_or_fail(this->get_symbols(), OperatorSequence{{1, 1}, this->get_context()});
            this->id_cc = find_or_fail(this->get_symbols(), OperatorSequence{{2, 2}, this->get_context()});
            this->id_aaa = find_or_fail(this->get_symbols(), OperatorSequence{{0, 0, 0}, this->get_context()});
            this->id_aab = find_or_fail(this->get_symbols(), OperatorSequence{{0, 0, 1}, this->get_context()});
            this->id_abb = find_or_fail(this->get_symbols(), OperatorSequence{{0, 1, 1}, this->get_context()});
            this->id_acc = find_or_fail(this->get_symbols(), OperatorSequence{{0, 2, 2}, this->get_context()});
            this->id_bbb = find_or_fail(this->get_symbols(), OperatorSequence{{1, 1, 1}, this->get_context()});
            this->id_bcc = find_or_fail(this->get_symbols(), OperatorSequence{{1, 2, 2}, this->get_context()});

            EXPECT_EQ(this->id_abb, factors.find_index_by_factors({this->id_a, this->id_bb}).value());
            EXPECT_EQ(this->id_acc, factors.find_index_by_factors({this->id_a, this->id_cc}).value());


            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ims_ptr->Symbols());


        }

        [[nodiscard]] const Inflation::InflationContext &get_context() const noexcept {
            return this->ims_ptr->InflationContext();
        };

        [[nodiscard]] const Inflation::FactorTable &get_factors() const noexcept {
            return this->ims_ptr->Factors();
        };

        [[nodiscard]] SymbolTable &get_symbols() noexcept {
            return this->ims_ptr->Symbols();
        };

        [[nodiscard]] Inflation::InflationMatrixSystem& get_system() noexcept {
            return *this->ims_ptr;
        };

        [[nodiscard]] const PolynomialFactory &get_factory() const noexcept {
            return *this->factory_ptr;
        };


    };


    TEST_F(Symbolic_MomentRulebook_Factor, Sub_AtoScalar) {
         // Prepare trivial rulebook
        auto& system = this->get_system();
        auto book_ptr = std::make_unique<MomentRulebook>(system, true);
        book_ptr->inject(this->id_a, Polynomial::Scalar(0.25)); // <A> = 0.25
        EXPECT_EQ(book_ptr->size(), 1);

        // Infer factored rules
        auto [index, book] = system.Rulebook.add(std::move(book_ptr));
        EXPECT_EQ(book.size(), 6);

        const auto& factory = this->get_factory();

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_a, 16.0}})),
                  Polynomial::Scalar(4.0)); // 16<A> -> 4

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_ab, 2.0}})),
                  factory({Monomial{this->id_b, 0.5}})); // 2<AB> -> 2<A><B> -> 0.5<B>

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_ac, 2.0}})),
                  factory({Monomial{this->id_c, 0.5}})); // 2<AC> -> 2<A><C> -> 0.5<C>

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_abc, 1.0}})),
                  factory({Monomial{this->id_bc, 0.25}})); // <ABC> -> <A><BC> -> 0.25<BC>

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_abb, 4.0}})),
                  factory({Monomial{this->id_bb, 1.0}})); // 4<ABB> -> 4<A><BB> -> <BB>

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_acc, 8.0}})),
                  factory({Monomial{this->id_cc, 2.0}})); // 8<ACC> -> 8<A><CC> -> 2<CC>


        // <A^3> should not change:
        EXPECT_EQ(book.reduce(factory({Monomial{this->id_aaa, 2.0}})),
                  factory({Monomial{this->id_aaa, 2.0}}));


    }

    TEST_F(Symbolic_MomentRulebook_Factor, Sub_BtoZero) {

        // Prepare trivial rulebook
        auto& system = this->get_system();
        auto book_ptr = std::make_unique<MomentRulebook>(system, true);
        book_ptr->inject(this->id_b, Polynomial()); // <B> = 0
        EXPECT_EQ(book_ptr->size(), 1);

        // Infer factored rules
        auto [index, book] = system.Rulebook.add(std::move(book_ptr));
        EXPECT_EQ(book.size(), 6);


        const auto& factory = this->get_factory();

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_b, 16.0}})),
                  Polynomial()); // 16<B> -> 0

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_ab, 2.0}})),
                  Polynomial()); // 2<AB> -> 2<A><B> -> 0

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bc, 2.0}})),
                  Polynomial()); // 2<BC> -> 2<B><C> -> 0

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_abc, 1.0}})),
                  Polynomial()); // <ABC> -> <A><B><C> -> 0

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_aab, 4.0}})),
                  Polynomial());

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bcc, 8.0}})),
                  Polynomial());


        // <B^3> should not change:
        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bbb, 2.0}})),
                  factory({Monomial{this->id_bbb, 2.0}}));

    }


    TEST_F(Symbolic_MomentRulebook_Factor, Sub_AandBtoScalar) {
        // Prepare trivial rulebook
        auto& system = this->get_system();
        auto book_ptr = std::make_unique<MomentRulebook>(system, true);
        book_ptr->inject(this->id_a, Polynomial::Scalar(0.3)); // <A> = 0.3
        book_ptr->inject(this->id_b, Polynomial::Scalar(0.4)); // <B> = 0.4
        book_ptr->complete();
        EXPECT_EQ(book_ptr->size(), 2);

        // Infer factored rules
        auto [index, book] = system.Rulebook.add(std::move(book_ptr));
        EXPECT_EQ(book.size(), 10);


        const auto& factory = this->get_factory();

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_a, 1.0}})),
                  Polynomial::Scalar(0.3));
        EXPECT_EQ(book.reduce(factory({Monomial{this->id_b, 1.0}})),
                  Polynomial::Scalar(0.4));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_ab, 2.0}})),
                  Polynomial::Scalar(0.24));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_ac, 1.0}})),
                  factory({Monomial{this->id_c, 0.3}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bc, 1.0}})),
                  factory({Monomial{this->id_c, 0.4}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_abc, 1.0}})),
                  factory({Monomial{this->id_c, 0.12}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_aab, 1.0}})),
                  factory({Monomial{this->id_aa, 0.4}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_abb, 1.0}})),
                  factory({Monomial{this->id_bb, 0.3}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_acc, 1.0}})),
                  factory({Monomial{this->id_cc, 0.3}}));

        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bcc, 1.0}})),
                  factory({Monomial{this->id_cc, 0.4}}));


        // <A^3> should not change:
        EXPECT_EQ(book.reduce(factory({Monomial{this->id_aaa, 2.0}})),
                  factory({Monomial{this->id_aaa, 2.0}}));
        // <B^3> should not change:
        EXPECT_EQ(book.reduce(factory({Monomial{this->id_bbb, 2.0}})),
                  factory({Monomial{this->id_bbb, 2.0}}));


    }

    TEST_F(Symbolic_MomentRulebook_Factor, RulesWithUpdate) {
        // TODO:
    }
}