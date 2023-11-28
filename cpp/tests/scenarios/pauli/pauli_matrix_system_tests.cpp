/**
 * pauli_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/monomial_matrix.h"

#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_moment_matrix.h"
#include "scenarios/pauli/pauli_localizing_matrix.h"

#include "../../matrix/compare_os_matrix.h"
#include "../../matrix/compare_symbol_matrix.h"

#include <complex>
#include <set>

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_MatrixSystem, Construct_Empty) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(0)};
        const auto& context = system.pauliContext;
        EXPECT_EQ(context.size(), 0);
        EXPECT_EQ(context.qubit_size, 0);
    }

    TEST(Scenarios_Pauli_MatrixSystem, Construct_TwoQubit) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(2)};
        const auto& context = system.pauliContext;
        EXPECT_EQ(context.size(), 6);
        EXPECT_EQ(context.qubit_size, 2);
    }

    TEST(Scenarios_Pauli_MatrixSystem, MomentMatrix_Qubit) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1, Multithreading::MultiThreadPolicy::Never);

        // Compare operator sequences
        compare_os_matrix(mmRaw, 4, {I, x, y, z,
                                     x, I, iz, miy,
                                     y, miz, I, ix,
                                     z, iy, mix, I});

        // Find symbols
        ASSERT_EQ(symbols.size(), 5); // 0, I, X, Y, Z

        const auto fX = symbols.where(x);
        ASSERT_TRUE(fX.found());
        auto sX = fX->Id();

        const auto fY = symbols.where(y);
        ASSERT_TRUE(fY.found());
        auto sY = fY->Id();

        const auto fZ = symbols.where(z);
        ASSERT_TRUE(fZ.found());
        const auto sZ = fZ->Id();

        std::set<symbol_name_t> symbol_set{0, 1, sX, sY, sZ};
        ASSERT_EQ(symbol_set.size(), 5);

        const std::complex<double> i{0.0, 1.0};

        compare_symbol_matrix(mmRaw, 4,
                              {Monomial{1, 1.0},  Monomial{sX, 1.0}, Monomial{sY, 1.0}, Monomial{sZ, 1.0},
                               Monomial{sX, 1.0}, Monomial{1, 1.0},  Monomial{sZ, i},   Monomial{sY, -i},
                               Monomial{sY, 1.0}, Monomial{sZ, -i},  Monomial{1, 1.0},  Monomial{sX, i},
                               Monomial{sZ, 1.0}, Monomial{sY, i},   Monomial{sX, -i},  Monomial{1, 1.0}});
    }

    TEST(Scenarios_Pauli_MatrixSystem, MomentMatrix_QubitMT) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1, Multithreading::MultiThreadPolicy::Always);

        // Compare operator sequences
        compare_os_matrix(mmRaw, 4, {I, x, y, z,
                                     x, I, iz, miy,
                                     y, miz, I, ix,
                                     z, iy, mix, I});

        // Find symbols
        ASSERT_EQ(symbols.size(), 5); // 0, I, X, Y, Z

        const auto fX = symbols.where(x);
        ASSERT_TRUE(fX.found());
        auto sX = fX->Id();

        const auto fY = symbols.where(y);
        ASSERT_TRUE(fY.found());
        auto sY = fY->Id();

        const auto fZ = symbols.where(z);
        ASSERT_TRUE(fZ.found());
        const auto sZ = fZ->Id();

        std::set<symbol_name_t> symbol_set{0, 1, sX, sY, sZ};
        ASSERT_EQ(symbol_set.size(), 5);

        const std::complex<double> i{0.0, 1.0};

        compare_symbol_matrix(mmRaw, 4,
                              {Monomial{1, 1.0},  Monomial{sX, 1.0}, Monomial{sY, 1.0}, Monomial{sZ, 1.0},
                               Monomial{sX, 1.0}, Monomial{1, 1.0},  Monomial{sZ, i},   Monomial{sY, -i},
                               Monomial{sY, 1.0}, Monomial{sZ, -i},  Monomial{1, 1.0},  Monomial{sX, i},
                               Monomial{sZ, 1.0}, Monomial{sY, i},   Monomial{sX, -i},  Monomial{1, 1.0}});
    }

    TEST(Scenarios_Pauli_MatrixSystem, FiveQubitSymbolTable) {
        // Test replicating weird bug found by Mateus whereby anti-Hermitian symbols are erroneously generated.

        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(5, false)};
        const auto& context = system.pauliContext;
        const auto& symbols = system.Symbols();

        const auto& mm = system.MomentMatrix(2);

        EXPECT_EQ(symbols.size(), 782);

        for (const auto& symbol : symbols) {
            EXPECT_TRUE(symbol.is_hermitian()) << "Symbol = " << symbol;
            ASSERT_TRUE(symbol.has_sequence()) << "Symbol = " << symbol;
            EXPECT_EQ(symbol.sequence().get_sign(), SequenceSignType::Positive) << "Symbol = " << symbol;
            EXPECT_EQ(symbol.sequence_conj().get_sign(), SequenceSignType::Positive) << "Symbol = " << symbol;
        }
    }

    TEST(Scenarios_Pauli_MatrixSystem, ThreeQubits_NearestNeighbourMM) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, false)};
        const auto& context = system.pauliContext;

        const auto& pMM = system.PauliMomentMatrices(NearestNeighbourIndex{2, 1});
        ASSERT_EQ(pMM.Dimension(), 28);
        ASSERT_TRUE(pMM.is_monomial());
        ASSERT_TRUE(pMM.has_operator_matrix());

        const auto* asMM_ptr = dynamic_cast<const PauliMomentMatrix*>(&(pMM.operator_matrix()));
        ASSERT_NE(asMM_ptr, nullptr);
        const auto& asMM = *asMM_ptr;

        EXPECT_EQ(asMM.Index, 2);
        EXPECT_EQ(asMM.NNIndex.moment_matrix_level, 2);
        EXPECT_EQ(asMM.NNIndex.neighbours, 1);

        const auto& pMM_alias = system.PauliMomentMatrices(NearestNeighbourIndex{2, 1});
        EXPECT_EQ(&pMM, &pMM_alias);

    }
    TEST(Scenarios_Pauli_MatrixSystem, ThreeQubits_NearestNeighbourLM) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, false)};
        const auto& context = system.pauliContext;
        EXPECT_FALSE(context.wrap);

        const auto x1 = context.sigmaX(0);
        const PauliLocalizingMatrixIndex plmi{2, 1, x1};

        const auto& pLM_x = system.PauliLocalizingMatrices(plmi);
        ASSERT_EQ(pLM_x.Dimension(), 28);
        ASSERT_TRUE(pLM_x.is_monomial());
        ASSERT_TRUE(pLM_x.has_operator_matrix());
        const auto* asLM_ptr = dynamic_cast<const PauliLocalizingMatrix*>(&pLM_x.operator_matrix());
        ASSERT_NE(asLM_ptr, nullptr);
        const auto& asLM = *asLM_ptr;

        EXPECT_EQ(asLM.Index.Level, 2);
        EXPECT_EQ(asLM.PauliIndex.Index.moment_matrix_level, 2);
        EXPECT_EQ(asLM.PauliIndex.Index.neighbours, 1);
        EXPECT_EQ(asLM.Index.Word, x1);

        const auto& pLM_x_alias = system.PauliLocalizingMatrices(plmi);
        EXPECT_EQ(&pLM_x, &pLM_x_alias);

    }

}