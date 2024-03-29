/**
 * polynomial_localizing_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "dictionary/raw_polynomial.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "matrix/polynomial_localizing_matrix.h"

namespace Moment::Tests {

    class Matrix_PolyLMTests : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ms_ptr;

    protected:
        symbol_name_t s_a, s_b, s_c;

    protected:
        void SetUp() override {
            // One party, two symbols
            this->ms_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(3));

            // Make basic symbols
            this->ms_ptr->generate_dictionary(2); // 0 1 a b c aa ab ac bb bc cc
            const auto &symbols = this->ms_ptr->Symbols();
            const auto &context = this->ms_ptr->Context();
            ASSERT_EQ(symbols.size(), 11);
            ASSERT_EQ(symbols.Basis.RealSymbolCount(), 10);
            ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 3); // 6 = ab, 7 = ac, 9 = bc
            ASSERT_FALSE(symbols[6].is_hermitian());
            ASSERT_FALSE(symbols[7].is_hermitian());
            ASSERT_FALSE(symbols[9].is_hermitian());

            this->s_a = symbols.where(OperatorSequence({0}, context))->Id();
            this->s_b = symbols.where(OperatorSequence({1}, context))->Id();
            this->s_c = symbols.where(OperatorSequence({2}, context))->Id();

        }

        [[nodiscard]] Algebraic::AlgebraicMatrixSystem &get_system() noexcept { return *this->ms_ptr; }

        [[nodiscard]] const Algebraic::AlgebraicContext &get_context() noexcept {
            return this->ms_ptr->AlgebraicContext();
        }

        [[nodiscard]] const SymbolTable &get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory &get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }
    };

    TEST_F(Matrix_PolyLMTests, MakeZero) {
        const auto &factory = this->get_system();
        auto &system = this->get_system();

        auto &plm = system.PolynomialLocalizingMatrix(PolynomialLocalizingMatrixIndex{1, Polynomial::Zero()});
        EXPECT_EQ(plm.Dimension(), 4);
        for (const auto &elem: plm.SymbolMatrix()) {
            EXPECT_TRUE(elem.empty());
        }
    }


    TEST_F(Matrix_PolyLMTests, MakeMonomial) {
        auto &system = this->get_system();
        const auto &factory = this->get_factory();
        const auto &context = this->get_context();

        auto &plm = system.PolynomialLocalizingMatrix(PolynomialLocalizingMatrixIndex{1, Polynomial(Monomial{s_a, -2.0})});

        const LocalizingMatrixIndex lmi_a_1{1, OperatorSequence{{0}, context}};

        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_a_1));
        const auto &lmA = dynamic_cast<const MonomialMatrix &>(system.LocalizingMatrix(lmi_a_1));

        ASSERT_EQ(plm.Dimension(), 4);
        ASSERT_EQ(lmA.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const auto &poly_elem = plm.SymbolMatrix(row, col);
                const auto &mono_elem = lmA.SymbolMatrix(row, col);
                ASSERT_EQ(poly_elem.size(), 1) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().id, mono_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().factor, -2.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().conjugated, mono_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
            }
        }
    }

    TEST_F(Matrix_PolyLMTests, MakePoly) {
        auto &system = this->get_system();
        const auto &factory = this->get_factory();
        const auto &context = this->get_context();

        const PolynomialLocalizingMatrixIndex plmIndex{1, factory({Monomial{s_a, -2.0}, Monomial{s_b, 1.0}})};
        auto &plm = system.PolynomialLocalizingMatrix(plmIndex);

        const LocalizingMatrixIndex lmi_a_1{1, OperatorSequence{{0}, context}};
        const LocalizingMatrixIndex lmi_b_1{1, OperatorSequence{{1}, context}};

        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_a_1));
        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_b_1));
        const auto &lmA = dynamic_cast<const MonomialMatrix &>(system.LocalizingMatrix(lmi_a_1));
        const auto &lmB = dynamic_cast<const MonomialMatrix &>(system.LocalizingMatrix(lmi_b_1));

        ASSERT_EQ(plm.Dimension(), 4);
        ASSERT_EQ(lmA.Dimension(), 4);
        ASSERT_EQ(lmB.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const auto &poly_elem = plm.SymbolMatrix(row, col);
                const auto &monoA_elem = lmA.SymbolMatrix(row, col);
                const auto &monoB_elem = lmB.SymbolMatrix(row, col);
                ASSERT_EQ(poly_elem.size(), 2) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[0].id, monoA_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[0].factor, -2.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[0].conjugated, monoA_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[1].id, monoB_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[1].factor, 1.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[1].conjugated, monoB_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
            }
        }
    }

    TEST_F(Matrix_PolyLMTests, MakeFromRaw) {
        auto &system = this->get_system();
        const auto& const_system = system;

        const auto &factory = this->get_factory();
        const auto &context = this->get_context();

        const OperatorSequence ccc{{4, 4, 4}, context};
        auto find_result_initial = system.Symbols().where(ccc);
        ASSERT_FALSE(find_result_initial.found());

        RawPolynomial raw_poly;
        raw_poly.emplace_back(ccc, std::complex{0.5, 0.0});
        EXPECT_EQ(raw_poly.size(), 1);

        auto [poly_ccc_offset, poly_mat] = system.create_and_register_localizing_matrix(1, raw_poly,
                                                                              Multithreading::MultiThreadPolicy::Never);
        EXPECT_EQ(poly_ccc_offset, 1); // ccc mono is matrix 0
        const auto* as_plm_ptr = dynamic_cast<const PolynomialLocalizingMatrix*>(&poly_mat);
        ASSERT_NE(as_plm_ptr, nullptr);

        auto find_result_made = system.Symbols().where(ccc);
        ASSERT_TRUE(find_result_made.found());

        // Validate index
        EXPECT_EQ(as_plm_ptr->index.Level, 1);
        EXPECT_EQ(as_plm_ptr->index.Polynomial.size(), 1);
        EXPECT_EQ(as_plm_ptr->index.Polynomial[0], Monomial(find_result_made->Id(), {0.5, 0.0}));

        const auto &lmCCC = dynamic_cast<const MonomialMatrix &>(const_system.LocalizingMatrix({1, ccc}));
        EXPECT_TRUE(lmCCC.has_aliased_operator_matrix());
        EXPECT_EQ(const_system.size(), 2); // ccc and 0.5 ccc

        ASSERT_EQ(as_plm_ptr->Constituents().size(), 1);
        EXPECT_EQ(as_plm_ptr->Constituents()[0].first, &lmCCC);
        EXPECT_EQ(as_plm_ptr->Constituents()[0].second, std::complex<double>(0.5, 0.0));
    }

    TEST_F(Matrix_PolyLMTests, IndexNotFound) {
        const auto& system = this->get_system();
        const auto& factory = this->get_factory();

        const PolynomialLocalizingMatrixIndex plmIndex{1, factory({Monomial{s_a, -2.0}, Monomial{s_b, 1.0}})};
        EXPECT_THROW([[maybe_unused]] const auto& mm = system.PolynomialLocalizingMatrix(plmIndex),
                     Moment::errors::missing_component);
    }
}