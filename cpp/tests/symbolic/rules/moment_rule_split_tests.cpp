/**
 * moment_rule_split_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/imported/imported_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "symbolic/rules/moment_rule.h"
#include "symbolic/monomial_comparator_by_hash.h"

#include "moment_rule_helpers.h"


namespace Moment::Tests {

    class Symbolic_MomentRule_Split : public ::testing::Test {
    private:
        std::unique_ptr<Imported::ImportedMatrixSystem> ims_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        symbol_name_t id, re_a, comp_b, im_c, re_d, comp_e, im_f;

    protected:
        void SetUp() override {

            ims_ptr = std::make_unique<Imported::ImportedMatrixSystem>();
            auto &symbols = ims_ptr->Symbols();
            symbols.create(1, true, false); // a = 2 real
            symbols.create(1, true, true); // b = 3  complex
            symbols.create(1, false, true); // c = 4 imaginary
            symbols.create(1, true, false); // a = 5 real
            symbols.create(1, true, true); // b = 6  complex
            symbols.create(1, false, true); // c = 7 imaginary

            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ims_ptr->Symbols(), 10.0);

            this->id = 1;
            this->re_a = 2;
            this->comp_b = 3;
            this->im_c = 4;
            this->re_d = 5;
            this->comp_e = 6;
            this->im_f = 7;

            // Real
            ASSERT_TRUE(symbols[this->re_a].is_hermitian());
            ASSERT_FALSE(symbols[this->re_a].is_antihermitian());
            ASSERT_TRUE(symbols[this->re_d].is_hermitian());
            ASSERT_FALSE(symbols[this->re_d].is_antihermitian());

            // Complex
            ASSERT_FALSE(symbols[this->comp_b].is_hermitian());
            ASSERT_FALSE(symbols[this->comp_b].is_antihermitian());
            ASSERT_FALSE(symbols[this->comp_e].is_hermitian());
            ASSERT_FALSE(symbols[this->comp_e].is_antihermitian());

            // Imaginary
            ASSERT_FALSE(symbols[this->im_c].is_hermitian());
            ASSERT_TRUE(symbols[this->im_c].is_antihermitian());
            ASSERT_FALSE(symbols[this->im_f].is_hermitian());
            ASSERT_TRUE(symbols[this->im_f].is_antihermitian());
        }

        [[nodiscard]] Imported::ImportedMatrixSystem& get_system() const noexcept {
            return *this->ims_ptr;
        }


        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ims_ptr->Symbols(); };

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept { return *this->factory_ptr; };

        void expect_approximately_equal(const Polynomial& LHS, const Polynomial& RHS) {
            expect_matching_polynomials("Polynomial", LHS, RHS, this->factory_ptr->zero_tolerance);
        }

    };

    TEST_F(Symbolic_MomentRule_Split, NoSplit_Trivial) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, Polynomial::Zero()};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), 0);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
    }

    TEST_F(Symbolic_MomentRule_Split, NoSplit_SimpleEqualsZero) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{comp_b, 1.0}})};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), comp_b);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
    }

    TEST_F(Symbolic_MomentRule_Split, NoSplit_SimpleEqualsNonzero) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{comp_b, 1.0}, Monomial{re_a, -1.0}})};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), comp_b);
        expect_approximately_equal(msr.RHS(), factory({Monomial{re_a, 1.0}}));
    }

    TEST_F(Symbolic_MomentRule_Split, NoSplit_HermitianEqualsScalar) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{re_a, 1.0}, Monomial{id, -5.0}})};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), re_a);
        expect_approximately_equal(msr.RHS(), factory({Monomial{id, 5.0}}));
    }

    TEST_F(Symbolic_MomentRule_Split, BadSplit_HermitianEqualsComplexScalar) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{re_a, 1.0}, Monomial{id, std::complex{0.0, -5.0}}})};
        auto split = msr.split();
        ASSERT_TRUE(split.has_value());
        EXPECT_EQ(msr.LHS(), re_a);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
        EXPECT_EQ(MomentRule::get_difficulty(split.value()),
                  MomentRule::PolynomialDifficulty::Contradiction); // Split rule is 0 = 5
    }


    TEST_F(Symbolic_MomentRule_Split, Split_HermitianEqualsComplex) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{re_d, 1.0}, Monomial{comp_b, -1.0}, Monomial{id, -1.0}})};
        auto split = msr.split();
        EXPECT_EQ(msr.LHS(), re_d);
        // d = Re(d) = Re(b) + 1
        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_b, 0.5}, Monomial{comp_b, 0.5, true}, Monomial{id, 1.0}}));
        // D -> Re(B) + 1
        ASSERT_TRUE(split.has_value()); // split should be Im(D) = 0 = Im(B) ->
        expect_approximately_equal(split.value(),
                                   factory({Monomial{comp_b, std::complex{0.0, -0.5}},
                                            Monomial{comp_b, std::complex{0.0, 0.5}, true}})); // Im(B) = 0.

        EXPECT_EQ(MomentRule::get_difficulty(split.value()),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
    }

    TEST_F(Symbolic_MomentRule_Split, Split_AntiHermitianEqualsComplex) {
        const auto& factory = this->get_factory();
        MomentRule msr{factory, factory({Monomial{im_f, 1.0}, Monomial{comp_b, -1.0}, Monomial{id, -1.0}})};
        auto split = msr.split();
        EXPECT_EQ(msr.LHS(), im_f);
        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_b, std::complex{0.5, 0.0}},
                                            Monomial{comp_b, std::complex{-0.5, 0.0}, true}}));// f = i Im(f) = i Im(b).
        // F -> Re(B) + 1
        ASSERT_TRUE(split.has_value()); // split should be Im(f) = 0 = Im(B) ->
        expect_approximately_equal(split.value(),
                                   factory({Monomial{comp_b, 0.5}, Monomial{comp_b, 0.5, true},
                                            Monomial{id, 1.0}})); // 0 = Re(f) = Re(b) + 1.

        EXPECT_EQ(MomentRule::get_difficulty(split.value()),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
    }

    TEST_F(Symbolic_MomentRule_Split, NonOrient_EasyConstraintOnReal) {
        const auto& factory = this->get_factory();
        auto rule_poly = factory({Monomial{comp_e, 0.5}, Monomial{comp_e, 0.5, true}, Monomial{id, -1.0}});  // Re(E) = 1
        ASSERT_EQ(MomentRule::get_difficulty(rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule msr{factory, std::move(rule_poly)};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());

        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{-0.5, 0.0}, true},
                                            Monomial{id, std::complex{1.0, 0.0}}}));

    }

    TEST_F(Symbolic_MomentRule_Split, NonOrient_ContradictoryConstraintOnReal) {
        const auto& factory = this->get_factory();
        auto rule_poly = factory({Monomial{comp_e, 0.5}, Monomial{comp_e, 0.5, true},
                                  Monomial{id, -std::complex{1.0, 1.0}}});  // Re(E) = 1 + 1i
        ASSERT_EQ(MomentRule::get_difficulty(rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule msr{factory, std::move(rule_poly)};
        ASSERT_TRUE(msr.is_partial());
        EXPECT_TRUE(approximately_equal(msr.partial_direction(), std::complex{1.0, 0.0}, factory.zero_tolerance));

        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{-0.5, 0.0}, true},
                                             Monomial{id, std::complex{1.0, 0.0}}}));

        auto split = msr.split();
        ASSERT_TRUE(split.has_value());
        EXPECT_EQ(MomentRule::get_difficulty(split.value()),
                  MomentRule::PolynomialDifficulty::Contradiction);
        EXPECT_EQ(split.value(), Polynomial::Scalar(std::complex{1.0, 0.0})); // Im of above gives: 0 = 1.
    }


    TEST_F(Symbolic_MomentRule_Split, NonOrient_ComplexConstraintOnReal) {
        const auto& factory = this->get_factory();
        // Re(E) = b + 5i; Re: Re(E) = Re(b); Im: 0 = Im(b) + 5 -> Im(b) = -5.
        auto rule_poly = factory({Monomial{comp_e, 0.5}, Monomial{comp_e, 0.5, true},
                                  Monomial{comp_b, -1.0}, Monomial{id, std::complex{0.0, -5.0}}});
        ASSERT_EQ(MomentRule::get_difficulty(rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule msr{factory, std::move(rule_poly)};
        ASSERT_TRUE(msr.is_partial());
        EXPECT_TRUE(approximately_equal(msr.partial_direction(), std::complex{1.0, 0.0}, factory.zero_tolerance));

        auto split = msr.split();

        // splits to Re(E) = Re(b), Im(b) = -5
        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{-0.5, 0.0}, true},
                                            Monomial{comp_b, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_b, std::complex{+0.5, 0.0}, true}})); // X -> iIm(X) + Re(Y)
        ASSERT_TRUE(split.has_value());
        EXPECT_EQ(MomentRule::get_difficulty(split.value()),
                  MomentRule::PolynomialDifficulty::NonorientableRule); // Im(b) = 5.
        expect_approximately_equal(split.value(), factory({Monomial{comp_b, std::complex{0.0, -0.5}, false},
                                                           Monomial{comp_b, std::complex{0.0, 0.5}, true},
                                                           Monomial{id, std::complex{5.0, 0.0}}}));

        auto second_rule = MomentRule{factory, std::move(split.value())};
        ASSERT_TRUE(second_rule.is_partial());
        EXPECT_TRUE(approximately_equal(second_rule.partial_direction(),
                                        std::complex{0.0, 1.0},
                                        factory.zero_tolerance))
                            << second_rule.partial_direction();

        // Im(b) = 5, so B -> Re(B) + 5i
        expect_approximately_equal(second_rule.RHS(),
                                   factory({Monomial{comp_b, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_b, std::complex{0.5, 0.0}, true},
                                            Monomial{id, std::complex{0.0, -5.0}}}));




        auto second_split = second_rule.split();
        EXPECT_FALSE(second_split.has_value()) << second_split.value();


    }

    TEST_F(Symbolic_MomentRule_Split, NonOrient_EasyConstraintOnImaginary) {
        const auto& factory = this->get_factory();
        auto rule_poly = factory({Monomial{comp_e, std::complex{0.0, -0.5}},
                                  Monomial{comp_e, std::complex{0.0, 0.5}, true},
                                  Monomial{id, -1.0}});  // Im(E) - 1 = 0; -> E = Re(E) + i
        ASSERT_EQ(MomentRule::get_difficulty(rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule msr{factory, std::move(rule_poly)};
        auto split = msr.split();
        EXPECT_FALSE(split.has_value());

        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{0.5, 0.0}, true},
                                            Monomial{id, std::complex{0.0, 1.0}}}));

    }

    TEST_F(Symbolic_MomentRule_Split, Merge_ImIntoRe) {
        const auto& factory = this->get_factory();
        auto re_rule_poly = factory({Monomial{comp_e, std::complex{0.5, 0.0}},
                                     Monomial{comp_e, std::complex{0.5, 0.0}, true},
                                     Monomial{id, -2.0}});  // Re(E) - 2 = 0; -> E = iIm(E) + 2
        ASSERT_EQ(MomentRule::get_difficulty(re_rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule re_rule{factory, std::move(re_rule_poly)};
        auto re_split = re_rule.split();
        EXPECT_FALSE(re_split.has_value());

        expect_approximately_equal(re_rule.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{-0.5, 0.0}, true},
                                            Monomial{id, std::complex{2.0, 0.0}}}));


        auto im_rule_poly = factory({Monomial{comp_e, std::complex{0.0, -0.5}},
                                     Monomial{comp_e, std::complex{0.0, 0.5}, true},
                                     Monomial{id, -3.0}});  // Im(E) - 3 = 0; -> E = Re(E) + 3i
        ASSERT_EQ(MomentRule::get_difficulty(im_rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule im_rule{factory, std::move(im_rule_poly)};
        auto im_split = im_rule.split();
        EXPECT_FALSE(im_split.has_value());
        expect_approximately_equal(im_rule.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{0.5, 0.0}, true},
                                            Monomial{id, std::complex{0.0, 3.0}}}));

        re_rule.merge_partial(factory, std::move(im_rule));
        ASSERT_FALSE(re_rule.is_partial());
        EXPECT_EQ(re_rule.LHS(), comp_e);
        EXPECT_EQ(re_rule.RHS(), factory({Monomial{1, std::complex{2.0, 3.0}}}));

    }
    TEST_F(Symbolic_MomentRule_Split, Merge_ReIntoIm) {
        const auto& factory = this->get_factory();
        auto re_rule_poly = factory({Monomial{comp_e, std::complex{0.5, 0.0}},
                                     Monomial{comp_e, std::complex{0.5, 0.0}, true},
                                     Monomial{id, -2.0}});  // Re(E) - 2 = 0; -> E = iIm(E) + 2
        ASSERT_EQ(MomentRule::get_difficulty(re_rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule re_rule{factory, std::move(re_rule_poly)};
        auto re_split = re_rule.split();
        EXPECT_FALSE(re_split.has_value());

        expect_approximately_equal(re_rule.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{-0.5, 0.0}, true},
                                            Monomial{id, std::complex{2.0, 0.0}}}));


        auto im_rule_poly = factory({Monomial{comp_e, std::complex{0.0, -0.5}},
                                     Monomial{comp_e, std::complex{0.0, 0.5}, true},
                                     Monomial{id, -3.0}});  // Im(E) - 3 = 0; -> E = Re(E) + 3i
        ASSERT_EQ(MomentRule::get_difficulty(im_rule_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);
        MomentRule im_rule{factory, std::move(im_rule_poly)};
        auto im_split = im_rule.split();
        EXPECT_FALSE(im_split.has_value());
        expect_approximately_equal(im_rule.RHS(),
                                   factory({Monomial{comp_e, std::complex{0.5, 0.0}, false},
                                            Monomial{comp_e, std::complex{0.5, 0.0}, true},
                                            Monomial{id, std::complex{0.0, 3.0}}}));

        im_rule.merge_partial(factory, std::move(re_rule));
        ASSERT_FALSE(im_rule.is_partial());
        EXPECT_EQ(im_rule.LHS(), comp_e);
        EXPECT_EQ(im_rule.RHS(), factory({Monomial{1, std::complex{2.0, 3.0}}}));

    }

}
