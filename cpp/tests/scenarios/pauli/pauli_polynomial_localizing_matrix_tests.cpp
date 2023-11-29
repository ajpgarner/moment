/**
 * polynomial_localizing_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "dictionary/raw_polynomial.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_polynomial_localizing_matrix.h"

#include "symbolic/symbol_table.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

namespace Moment::Tests {
    using namespace Moment::Pauli;

    class Scenarios_Pauli_PolyLMTests : public ::testing::Test {
    private:
        std::unique_ptr<Pauli::PauliMatrixSystem> ms_ptr;

    protected:
        symbol_name_t sid_X, sid_Y, sid_Z;

    protected:
        void SetUp() override {
            // One party, two symbols
            this->ms_ptr = std::make_unique<Pauli::PauliMatrixSystem>(std::make_unique<Pauli::PauliContext>(1));

            // Make basic symbols
            this->ms_ptr->generate_dictionary(1);
            const auto& symbols = this->ms_ptr->Symbols();
            const auto& context = this->ms_ptr->pauliContext;


            this->sid_X = symbols.where(context.sigmaX(0))->Id();
            this->sid_Y = symbols.where(context.sigmaY(0))->Id();
            this->sid_Z = symbols.where(context.sigmaZ(0))->Id();
        }

        [[nodiscard]] Pauli::PauliMatrixSystem& get_system() noexcept { return *this->ms_ptr; }

        [[nodiscard]] const Pauli::PauliContext& get_context() noexcept {
            return this->ms_ptr->pauliContext;
        }

        [[nodiscard]] const SymbolTable& get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory& get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }
    };

    TEST_F(Scenarios_Pauli_PolyLMTests, Plain_MakeZero) {
        auto& system = this->get_system();

        auto& plm = system.PolynomialLocalizingMatrix(PolynomialLMIndex{1, Polynomial::Zero()});
        EXPECT_EQ(plm.Dimension(), 4);
        for (const auto& elem: plm.SymbolMatrix()) {
            EXPECT_TRUE(elem.empty());
        }
    }


    TEST_F(Scenarios_Pauli_PolyLMTests, Plain_MakeMonomial) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const auto& context = this->get_context();

        auto& plm = system.PolynomialLocalizingMatrix(PolynomialLMIndex{1, Polynomial(Monomial{sid_X, -2.0})});

        const LocalizingMatrixIndex lmi_a_1{1, OperatorSequence{{0}, context}};

        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_a_1));
        const auto& lmA = dynamic_cast<const MonomialMatrix&>(system.LocalizingMatrix(lmi_a_1));

        ASSERT_EQ(plm.Dimension(), 4);
        ASSERT_EQ(lmA.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const auto& poly_elem = plm.SymbolMatrix(row, col);
                const auto& mono_elem = lmA.SymbolMatrix(row, col);
                ASSERT_EQ(poly_elem.size(), 1) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().id, mono_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().factor, mono_elem.factor * -2.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem.back().conjugated, mono_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
            }
        }
    }

    TEST_F(Scenarios_Pauli_PolyLMTests, Plain_MakePoly) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const auto& context = this->get_context();

        const PolynomialLMIndex plmIndex{1, factory({Monomial{sid_X, -2.0}, Monomial{sid_Y, 1.0}})};
        auto& plm = system.PolynomialLocalizingMatrix(plmIndex);

        const LocalizingMatrixIndex lmi_a_1{1, OperatorSequence{{0}, context}};
        const LocalizingMatrixIndex lmi_b_1{1, OperatorSequence{{1}, context}};

        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_a_1));
        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_b_1));
        const auto& lmA = dynamic_cast<const MonomialMatrix&>(system.LocalizingMatrix(lmi_a_1));
        const auto& lmB = dynamic_cast<const MonomialMatrix&>(system.LocalizingMatrix(lmi_b_1));

        ASSERT_EQ(plm.Dimension(), 4);
        ASSERT_EQ(lmA.Dimension(), 4);
        ASSERT_EQ(lmB.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const auto& poly_elem = plm.SymbolMatrix(row, col);
                const auto& monoA_elem = lmA.SymbolMatrix(row, col);
                const auto& monoB_elem = lmB.SymbolMatrix(row, col);
                const bool swap_order = monoB_elem.id < monoA_elem.id;

                ASSERT_EQ(poly_elem.size(), 2) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].id, monoA_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].factor, monoA_elem.factor*-2.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].conjugated, monoA_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].id, monoB_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].factor, monoB_elem.factor) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].conjugated, monoB_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
            }
        }
    }

    TEST_F(Scenarios_Pauli_PolyLMTests, Plain_MakePolyRaw) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const auto& context = this->get_context();

        RawPolynomial raw_poly;
        raw_poly.emplace_back(context.sigmaX(0), std::complex{-2.0, 0.0});
        raw_poly.emplace_back(context.sigmaY(0), std::complex{1.0, 0.0});
        ASSERT_EQ(raw_poly.size(), 2);

        auto [offset, plm] = system.create_and_register_localizing_matrix(NearestNeighbourIndex{1, 0}, raw_poly);
        ASSERT_FALSE(plm.is_monomial());

        const LocalizingMatrixIndex lmi_a_1{1, OperatorSequence{{0}, context}};
        const LocalizingMatrixIndex lmi_b_1{1, OperatorSequence{{1}, context}};

        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_a_1));
        ASSERT_TRUE(system.LocalizingMatrix.contains(lmi_b_1));
        const auto& lmA = dynamic_cast<const MonomialMatrix&>(system.LocalizingMatrix(lmi_a_1));
        const auto& lmB = dynamic_cast<const MonomialMatrix&>(system.LocalizingMatrix(lmi_b_1));

        ASSERT_EQ(plm.Dimension(), 4);
        ASSERT_EQ(lmA.Dimension(), 4);
        ASSERT_EQ(lmB.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const auto& poly_elem = plm.SymbolMatrix(row, col);
                const auto& monoA_elem = lmA.SymbolMatrix(row, col);
                const auto& monoB_elem = lmB.SymbolMatrix(row, col);
                const bool swap_order = monoB_elem.id < monoA_elem.id;

                ASSERT_EQ(poly_elem.size(), 2) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].id, monoA_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].factor, monoA_elem.factor*-2.0) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 1 : 0].conjugated, monoA_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].id, monoB_elem.id) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].factor, monoB_elem.factor) << "col = " << col << ", row = " << row;
                EXPECT_EQ(poly_elem[swap_order ? 0 : 1].conjugated, monoB_elem.conjugated)
                                    << "col = " << col << ", row = " << row;
            }
        }
    }
}