/**
 * commutator_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "../../matrix/compare_os_matrix.h"
#include "../../matrix/compare_symbol_matrix.h"

#include "scenarios/pauli/commutator_matrix.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

namespace Moment::Tests {
    using namespace Pauli;

    class Scenarios_Pauli_CommutatorMatrixTests : public ::testing::Test {
    private:
        std::unique_ptr<Pauli::PauliMatrixSystem> ms_ptr;
        const Pauli::PauliContext& context;

    protected:
        OperatorSequence Zero,
                         I, iI, mI, miI,
                         x, ix, mx, mix,
                         y, iy, my, miy,
                         z, iz, mz, miz;

        symbol_name_t sI, sX, sY, sZ;

    public:
        Scenarios_Pauli_CommutatorMatrixTests() : ::testing::Test{},
                  ms_ptr{std::make_unique<Pauli::PauliMatrixSystem>(std::make_unique<Pauli::PauliContext>(1))},
                  context{ms_ptr->pauliContext},
                  Zero{context.zero()},
                  I{OperatorSequence{context}},
                  iI{OperatorSequence{{}, context, SequenceSignType::Imaginary}},
                  mI{OperatorSequence{{}, context, SequenceSignType::Negative}},
                  miI{OperatorSequence{{}, context, SequenceSignType::NegativeImaginary}},
                  x{context.sigmaX(0)},
                  ix{context.sigmaX(0, SequenceSignType::Imaginary)},
                  mx{context.sigmaX(0, SequenceSignType::Negative)},
                  mix{context.sigmaX(0, SequenceSignType::NegativeImaginary)},
                  y{context.sigmaY(0)},
                  iy{context.sigmaY(0, SequenceSignType::Imaginary)},
                  my{context.sigmaY(0, SequenceSignType::Negative)},
                  miy{context.sigmaY(0, SequenceSignType::NegativeImaginary)},
                  z{context.sigmaZ(0)},
                  iz{context.sigmaZ(0, SequenceSignType::Imaginary)},
                  mz{context.sigmaZ(0, SequenceSignType::Negative)},
                  miz{context.sigmaZ(0, SequenceSignType::NegativeImaginary)}
        {

            // Make basic symbols
            this->ms_ptr->generate_dictionary(1);
            auto& symbols = ms_ptr->Symbols();
            this->sI = 1;
            this->sX = symbols.where(x)->Id();
            this->sY = symbols.where(y)->Id();
            this->sZ = symbols.where(z)->Id();
        }

    protected:
        void SetUp() override { }

        [[nodiscard]] Pauli::PauliMatrixSystem& get_system() noexcept { return *this->ms_ptr; }

        [[nodiscard]] const Pauli::PauliContext& get_context() noexcept {
            return this->ms_ptr->pauliContext;
        }

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory& get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }
    };

    TEST_F(Scenarios_Pauli_CommutatorMatrixTests, Commute_Z1) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const auto& context = this->get_context();
        auto& symbols = this->get_symbols();
        const PauliLocalizingMatrixIndex plmi_Z1{1, 0, context.sigmaZ(0)}; // [MM, Z1]

        auto& matrix = system.CommutatorMatrices(plmi_Z1);

        ASSERT_TRUE(matrix.is_monomial());
        const auto& mono_matrix = dynamic_cast<const MonomialMatrix&>(matrix);
        ASSERT_TRUE(matrix.has_operator_matrix());
        ASSERT_NE(MonomialCommutatorMatrix::to_operator_matrix_ptr(matrix), nullptr);
        const auto& op_matrix = mono_matrix.operator_matrix();
        compare_os_matrix("[MM, Z]", op_matrix, 4, {Zero,  miy,   ix,   Zero,
                                                    miy,   Zero,  Zero, x,
                                                    ix,    Zero,  Zero, y,
                                                    Zero,  mx,    my,   Zero});

        compare_monomial_matrix("[MM, Z]", mono_matrix, 4,
                                {Monomial{0}, Monomial{sY, {0.0, -2.0}}, Monomial{sX, {0.0, 2.0}}, Monomial{0},
                                 Monomial{sY, {0.0, -2.0}}, Monomial{0}, Monomial{0},  Monomial{sX, {2.0, 0.0}},
                                 Monomial{sX, {0.0, 2.0}}, Monomial{0}, Monomial{0},  Monomial{sY, {2.0, 0.0}},
                                 Monomial{0}, Monomial{sX, {-2.0, 0.0}}, Monomial{sY, {-2.0, 0.0}}, Monomial{0}});

    }

    TEST_F(Scenarios_Pauli_CommutatorMatrixTests, Anticommute_Z1) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const auto& context = this->get_context();
        auto& symbols = this->get_symbols();
        const PauliLocalizingMatrixIndex plmi_Z1{1, 0, context.sigmaZ(0)}; // [MM, Z1]

        auto& matrix = system.AnticommutatorMatrices(plmi_Z1);
        ASSERT_TRUE(matrix.is_monomial());
        const auto& mono_matrix = dynamic_cast<const MonomialMatrix&>(matrix);
        ASSERT_TRUE(matrix.has_operator_matrix());
        ASSERT_NE(MonomialAnticommutatorMatrix::to_operator_matrix_ptr(matrix), nullptr);
        const auto& op_matrix = mono_matrix.operator_matrix();
        compare_os_matrix("{MM, Z}", op_matrix, 4, {z,    Zero,   Zero, I,
                                                    Zero, z,      iI,   Zero,
                                                    Zero, miI,    z,    Zero,
                                                    I,    Zero,   Zero, z});


        compare_monomial_matrix("{MM, Z}", mono_matrix, 4,
                                {Monomial{sZ, 2.0}, Monomial{0}, Monomial{0}, Monomial{1, 2.0},
                                 Monomial{0}, Monomial{sZ, 2.0}, Monomial{1, {0.0, 2.0}}, Monomial{0},
                                 Monomial{0}, Monomial{1, {0.0, -2.0}}, Monomial{sZ, 2.0}, Monomial{0},
                                 Monomial{1, 2.0}, Monomial{0}, Monomial{0}, Monomial{sZ, 2.0}});

    }

    TEST_F(Scenarios_Pauli_CommutatorMatrixTests, Commute_X1_plus_Z1) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const Polynomial X1_plus_Z1{factory({Monomial{sX, 1.0}, Monomial{sZ, 1.0}})};

        const PolynomialCommutatorMatrixIndex cmi_x1_plus_z1{1, 0, X1_plus_Z1}; // [MM, X1 + Z1]

        auto& matrix = system.PolynomialCommutatorMatrices(cmi_x1_plus_z1);
        ASSERT_FALSE(matrix.is_monomial());
        const auto& poly_matrix = dynamic_cast<const PolynomialMatrix&>(matrix);

        compare_polynomial_matrix("[MM, X+Z]", poly_matrix, 4, factory.zero_tolerance, {
               factory({Monomial{0}}),
               factory({Monomial{sY, {0.0, -2.0}}}),
               factory({Monomial{sZ, {0.0, -2.0}}, Monomial{sX, {0.0, 2.0}}}),
               factory({Monomial{sY, {0.0,  2.0}}}),

               factory({Monomial{sY, {0.0, -2.0}}}),
               factory({Monomial{0}}),
               factory({Monomial{sY, -2.0}}),
               factory({Monomial{sZ, -2.0}, Monomial{sX, 2.0}}),

               factory({Monomial{sZ, {0.0, -2.0}}, Monomial{sX, {0.0, 2.0}}}),
               factory({Monomial{sY, 2.0}}),
               factory({Monomial{0}}),
               factory({Monomial{sY, 2.0}}),

               factory({Monomial{sY, {0.0, 2.0}}}),
               factory({Monomial{sZ, 2.0}, Monomial{sX, -2.0}}),
               factory({Monomial{sY, -2.0}}),
               factory({Monomial{0}})
      });
    }

    TEST_F(Scenarios_Pauli_CommutatorMatrixTests, Anticommute_X1_plus_Z1) {
        auto& system = this->get_system();
        const auto& factory = this->get_factory();
        const Polynomial X1_plus_Z1{factory({Monomial{sX, 1.0}, Monomial{sZ, 1.0}})};

        const PolynomialCommutatorMatrixIndex cmi_x1_plus_z1{1, 0, X1_plus_Z1}; // [MM, X1 + Z1]

        auto& matrix = system.PolynomialAnticommutatorMatrices(cmi_x1_plus_z1);
        ASSERT_FALSE(matrix.is_monomial());
        const auto& poly_matrix = dynamic_cast<const PolynomialMatrix&>(matrix);

        compare_polynomial_matrix("{MM, X+Z}", poly_matrix, 4, factory.zero_tolerance,
              {factory({Monomial{sX, 2.0}, Monomial{sZ, 2.0}}),
               factory({Monomial{1, 2.0}}),
               factory({Monomial{0}}),
               factory({Monomial{1, 2.0}}),

               factory({Monomial{1, 2.0}}),
               factory({Monomial{sX, 2.0}, Monomial{sZ, 2.0}}),
               factory({Monomial{1, {0.0, 2.0}}}),
               factory({Monomial{0}}),

               factory({Monomial{0}}),
               factory({Monomial{1, {0.0, -2.0}}}),
               factory({Monomial{sX, 2.0}, Monomial{sZ, 2.0}}),
               factory({Monomial{1, {0.0, 2.0}}}),

               factory({Monomial{1, 2.0}}),
               factory({Monomial{0}}),
               factory({Monomial{1, {0.0, -2.0}}}),
               factory({Monomial{sX, 2.0}, Monomial{sZ, 2.0}})});
    }

}