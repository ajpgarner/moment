/**
 * moment_substitution_rulebook_to_basis_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix_system.h"

#include "scenarios/context.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/rules/moment_rulebook_to_basis.h"

#include "../symbolic_matrix_helpers.h"

#include "../../scenarios/sparse_utils.h"

#include <memory>
#include <numbers>

namespace Moment::Tests {

    class Symbolic_MomentRulebookToBasis : public ::testing::Test {
    private:
        std::unique_ptr<MatrixSystem> ms_ptr;

    protected:
        size_t total_symbol_count = 0;

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

            this->total_symbol_count = symbols.Basis.RealSymbolCount() + symbols.Basis.ImaginarySymbolCount();
        }

        [[nodiscard]] MatrixSystem& get_system() noexcept { return *this->ms_ptr; }

        [[nodiscard]] SymbolTable &get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory& get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }

        [[nodiscard]] inline const MomentRulebookToBasis get_mrtb(const bool homogenous = false) noexcept {
            return MomentRulebookToBasis{this->ms_ptr->polynomial_factory(),
                                         homogenous ? MomentRulebookToBasis::ExportMode::Homogeneous
                                                    : MomentRulebookToBasis::ExportMode::Rewrite};
        }

        void compare_sparse_matrices(const std::string& label,
                                     const MomentRulebookToBasis::output_t& actual,
                                     const MomentRulebookToBasis::output_t& expected) {
            ASSERT_EQ(actual.cols(), expected.cols()) << label;
            ASSERT_EQ(actual.rows(), expected.rows()) << label;
            ASSERT_EQ(actual.nonZeros(), expected.nonZeros()) << label;
            for (Eigen::Index outer_index = 0; outer_index < expected.cols(); ++outer_index) {
                for (auto iter = MomentRulebookToBasis::output_t::InnerIterator(actual, outer_index); iter; ++iter) {
                    EXPECT_FLOAT_EQ(iter.value(), expected.coeff(iter.row(), iter.col())) << label
                        << ", Index = " << iter.index() << ", Row = " << iter.row() << ", Col = " << iter.col();
                }
            }
        }

        void expect_idempotent(const MomentRulebookToBasis::output_t& actual, std::string label = "Idempotence") {
            auto squared_matrix = actual * actual;
            compare_sparse_matrices(label, actual, squared_matrix);
        }
    };

    TEST_F(Symbolic_MomentRulebookToBasis, Empty) {

        const auto& symbols = this->get_symbols();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        compare_sparse_matrices("Output", monolith, sparse_id<double>(this->total_symbol_count));
        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, RealToMonoScalar) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(3, factory({Monomial{1, 1.0}})); // <B> = 1.

        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 2) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }

        EXPECT_EQ(monolith.coeff(2, 0), 1.0);
        EXPECT_EQ(monolith.coeff(2, 2), 0.0);
        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, RealToPolynomial) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(3, factory({Monomial{2, -2.0}, Monomial{1, 1.0}})); // <B> = <A> + 1.

        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count+1);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 2) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), std::complex<double>(1.0, 0.0)) << "Index = " << idx;
        }

        EXPECT_EQ(monolith.coeff(2, 0), 1.0);
        EXPECT_EQ(monolith.coeff(2, 1), -2.0);

        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, ComplexToMonoScalar) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(6, factory({Monomial{1, std::complex{1.0, 2.0}}})); // <AB> = 1+2i.

        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if ((idx == 5) || (idx == 10)) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }

        EXPECT_EQ(monolith.coeff(5, 0), 1.0);
        EXPECT_EQ(monolith.coeff(10, 0), 2.0);

        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, ComplexToComplexMono) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(7, factory({Monomial{6, std::complex{0.0, 1.0}}})); // <AC> = i<AB>

        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if ((idx == 6) || (idx == 11)) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }

        ASSERT_EQ(symbols[6].basis_key().first, 5);
        ASSERT_EQ(symbols[6].basis_key().second, 0); // + 10 = 10

        ASSERT_EQ(symbols[7].basis_key().first, 6);
        ASSERT_EQ(symbols[7].basis_key().second, 1); // + 10 = 11

        EXPECT_EQ(monolith.coeff(6, 10), -1.0)  << monolith; // Re(<AC>) = -Im(<AB>)
        EXPECT_EQ(monolith.coeff(11, 5), 1.0); // Im(<AC>) = Re(<AB>)

        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, ConstrainRealPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(factory, 7, std::complex{1.0, 0.0}, factory({Monomial{2, 3.0}})); // Re(<AC>) = 3<A>
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 6) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }
        EXPECT_EQ(monolith.coeff(6, 1), 3.0);

        expect_idempotent(monolith);
    }

    TEST_F(Symbolic_MomentRulebookToBasis, ConstrainImaginaryPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        MomentRulebook rulebook{this->get_system()};
        // Im(<AC>) = 2<B>
        rulebook.inject(factory, 7, std::complex{0.0, 1.0}, factory({Monomial{3, 2.0}}));
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 11) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }
        EXPECT_EQ(monolith.coeff(11, 2), 2.0);

        expect_idempotent(monolith);
    }


    TEST_F(Symbolic_MomentRulebookToBasis, ConstrainSkewPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        const std::complex skew{std::numbers::sqrt2 / 2.0, std::numbers::sqrt2 / 2.0};

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(factory, 7, skew, factory({Monomial{1, -1.0}, Monomial{2, 3.0}})); // Sk(<AC>) = 3<A> - 1.
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count + 2);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 6) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }
        EXPECT_FLOAT_EQ(monolith.coeff(6, 0), -std::numbers::sqrt2);
        EXPECT_FLOAT_EQ(monolith.coeff(6, 1), 3.0 * std::numbers::sqrt2);
        EXPECT_FLOAT_EQ(monolith.coeff(6, 11), -1.0);
        expect_idempotent(monolith);

    }

    TEST_F(Symbolic_MomentRulebookToBasis, ConstrainMostlyImaginaryPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb();

        const std::complex skew{0.5, std::numbers::sqrt3/2.0}; // delta = pi / 3

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(factory, 7, skew, factory({Monomial{1, -1.0}, Monomial{2, 3.0}})); // Sk(<AC>) = 3<A> - 1.
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), this->total_symbol_count + 2);

        for (auto idx = 0; idx < this->total_symbol_count; ++idx) {
            if (idx == 11) {
                continue;
            }
            EXPECT_EQ(monolith.coeff(idx, idx), 1.0) << "Index = " << idx;
        }
        EXPECT_FLOAT_EQ(monolith.coeff(11, 0), -2.0 * std::numbers::inv_sqrt3);
        EXPECT_FLOAT_EQ(monolith.coeff(11, 1), 6.0 * std::numbers::inv_sqrt3);
        EXPECT_FLOAT_EQ(monolith.coeff(11, 6), -std::numbers::inv_sqrt3);
        expect_idempotent(monolith);

    }

    TEST_F(Symbolic_MomentRulebookToBasis, Homogenous_Empty) {

        const auto& symbols = this->get_symbols();
        const auto mrtb = this->get_mrtb(true);

        MomentRulebook rulebook{this->get_system()};
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), 0);
    }


    TEST_F(Symbolic_MomentRulebookToBasis, Homogenous_RealToPolynomial) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb(true);

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(3, factory({Monomial{2, -2.0}, Monomial{1, 1.0}})); // <B> = <A> + 1.

        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), 3);

        EXPECT_EQ(monolith.coeff(2, 0), 1.0);
        EXPECT_EQ(monolith.coeff(2, 1), -2.0);
        EXPECT_EQ(monolith.coeff(2, 2), -1.0);
    }


    TEST_F(Symbolic_MomentRulebookToBasis, Homogenous_ConstrainRealPart) {
        const auto& symbols = this->get_symbols();
        const auto& factory = this->get_factory();
        const auto mrtb = this->get_mrtb(true);

        MomentRulebook rulebook{this->get_system()};
        rulebook.inject(factory, 7, std::complex{1.0, 0.0}, factory({Monomial{2, 3.0}})); // Re(<AC>) = 3<A>
        auto monolith = mrtb(rulebook);

        EXPECT_EQ(monolith.cols(), this->total_symbol_count);
        EXPECT_EQ(monolith.rows(), this->total_symbol_count);
        EXPECT_EQ(monolith.nonZeros(), 2);

        EXPECT_EQ(monolith.coeff(6, 1), 3.0);
        EXPECT_EQ(monolith.coeff(6, 6), -1);

    }

}