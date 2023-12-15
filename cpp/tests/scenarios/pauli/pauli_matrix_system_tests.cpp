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
#include "scenarios/pauli/matrices/moment_matrix.h"
#include "scenarios/pauli/matrices/monomial_localizing_matrix.h"

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

        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(5, WrapType::None)};
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
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const auto& context = system.pauliContext;

        const auto& pMM = system.PauliMomentMatrices(Pauli::MomentMatrixIndex{2, 1});
        ASSERT_EQ(pMM.Dimension(), 28);
        ASSERT_TRUE(pMM.is_monomial());
        ASSERT_TRUE(pMM.has_operator_matrix());

        const auto* asMM_ptr = dynamic_cast<const Pauli::MomentMatrix*>(&(pMM.operator_matrix()));
        ASSERT_NE(asMM_ptr, nullptr);
        const auto& asMM = *asMM_ptr;

        EXPECT_EQ(asMM.Index.moment_matrix_level, 2);
        EXPECT_EQ(asMM.Index.neighbours, 1);

        const auto& pMM_alias = system.PauliMomentMatrices(Pauli::MomentMatrixIndex{2, 1});
        EXPECT_EQ(&pMM, &pMM_alias);

    }
    TEST(Scenarios_Pauli_MatrixSystem, ThreeQubits_NearestNeighbourLM) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const auto& context = system.pauliContext;
        ASSERT_EQ(context.wrap, WrapType::None);

        const auto x1 = context.sigmaX(0);
        const Pauli::LocalizingMatrixIndex plmi{NearestNeighbourIndex{2, 1}, x1};

        const auto& pLM_x = system.PauliLocalizingMatrices(plmi);
        ASSERT_EQ(pLM_x.Dimension(), 28);
        ASSERT_TRUE(pLM_x.is_monomial());
        ASSERT_TRUE(pLM_x.has_operator_matrix());
        const auto* asLM_ptr = dynamic_cast<const Pauli::MonomialLocalizingMatrix*>(&pLM_x.operator_matrix());
        ASSERT_NE(asLM_ptr, nullptr);
        const auto& asLM = *asLM_ptr;

        EXPECT_EQ(asLM.Index.Index.moment_matrix_level, 2);
        EXPECT_EQ(asLM.Index.Index.neighbours, 1);
        EXPECT_EQ(asLM.Index.Word, x1);

        const auto& pLM_x_alias = system.PauliLocalizingMatrices(plmi);
        EXPECT_EQ(&pLM_x, &pLM_x_alias);

    }


    TEST(Scenarios_Pauli_MatrixSystem, NotFound_PauliMomentMatrix) {
        const PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const Pauli::MomentMatrixIndex missing_index{2, 2};
        EXPECT_THROW([[maybe_unused]] const auto& mm = system.PauliMomentMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_PauliLocalizingMatrix) {
        const PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const Pauli::LocalizingMatrixIndex missing_index{2, 2, system.pauliContext.sigmaX(1)};
        EXPECT_THROW([[maybe_unused]] const auto& mm = system.PauliLocalizingMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_MonomialCommutator) {
        const PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const Pauli::CommutatorMatrixIndex missing_index{2, 2, system.pauliContext.sigmaX(1)};
        EXPECT_THROW([[maybe_unused]] const auto& mm = system.CommutatorMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_MonomialAnticommutator) {
        const PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const Pauli::AnticommutatorMatrixIndex missing_index{2, 2, system.pauliContext.sigmaX(1)};
        EXPECT_THROW([[maybe_unused]] const auto& mm = system.AnticommutatorMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_PolynomialLocalizingMatrix) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const auto& const_system = system;
        system.generate_dictionary(1);
        auto sX1 = system.Symbols().where(system.pauliContext.sigmaX(1));
        const Pauli::PolynomialLocalizingMatrixIndex missing_index{NearestNeighbourIndex{2, 2},
                                                                   Polynomial{Monomial{sX1->Id(), 1.0}}};
        EXPECT_THROW([[maybe_unused]] const auto& mm = const_system.PauliPolynomialLocalizingMatrices(missing_index),
                     Moment::errors::missing_component);

    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_PolynomialCommutator) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const auto& const_system = system;
        system.generate_dictionary(1);
        auto sX1 = system.Symbols().where(system.pauliContext.sigmaX(1));
        const Pauli::PolynomialCommutatorMatrixIndex missing_index{NearestNeighbourIndex{2, 2},
                                                                   Polynomial{Monomial{sX1->Id(), 1.0}}};
        EXPECT_THROW([[maybe_unused]] const auto& mm = const_system.PolynomialCommutatorMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, NotFound_PolynomialAnticommutator) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(3, WrapType::None)};
        const auto& const_system = system;
        system.generate_dictionary(1);
        auto sX1 = system.Symbols().where(system.pauliContext.sigmaX(1));
        const Pauli::PolynomialAnticommutatorMatrixIndex missing_index{NearestNeighbourIndex{2, 2},
                                                                       Polynomial{Monomial{sX1->Id(), 1.0}}};
        EXPECT_THROW([[maybe_unused]] const auto& mm = const_system.PolynomialAnticommutatorMatrices(missing_index),
                     Moment::errors::missing_component);
    }

    TEST(Scenarios_Pauli_MatrixSystem, Aliased_PolyLocalizingMatrix) {
        PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(4, WrapType::None, SymmetryType::Translational)};
        const auto& context = system.pauliContext;
        ASSERT_EQ(context.wrap, WrapType::None);
        ASSERT_EQ(context.translational_symmetry, SymmetryType::Translational);

        system.generate_dictionary(1); // Generate symbols for X, Y and Z
        EXPECT_EQ(system.Symbols().size(), 5); // 0, 1, X, Y and Z



        // Get two aliased operators
        const auto x1 = context.sigmaX(0);
        const auto x2 = context.sigmaX(1);
        EXPECT_EQ(context.simplify_as_moment(OperatorSequence{x2}), x1);


        auto sX1_res = system.Symbols().where(x1);
        ASSERT_TRUE(sX1_res.found());
        ASSERT_FALSE(sX1_res.is_aliased);
        auto sX2_res = system.Symbols().where(x2);
        ASSERT_TRUE(sX2_res.found());
        EXPECT_TRUE(sX2_res.is_aliased);
        EXPECT_EQ(sX1_res->Id(), sX2_res->Id());
        const symbol_name_t sX = sX1_res->Id();


        // Aliased raw polynomial
        RawPolynomial raw_poly;
        raw_poly.emplace_back(x1, 0.5);
        raw_poly.emplace_back(x2, 0.5);

        // Check aliasing resolves to just single moment
        const auto& factory = system.polynomial_factory();
        auto pure_poly = raw_poly.to_polynomial(factory);
        ASSERT_EQ(pure_poly.size(), 1);
        EXPECT_EQ(pure_poly[0].factor, 1.0);

        // Make aliased LM
        const auto& [matrix_id, matrix] = system.create_and_register_localizing_matrix(NearestNeighbourIndex{1,0},
                                                                                       raw_poly);


        // X1X2 should have been defined in the process...
        auto sX1X2_res = system.Symbols().where(x1*x2);
        ASSERT_TRUE(sX1X2_res.found());
        ASSERT_FALSE(sX1X2_res.is_aliased);
        const symbol_name_t sX1X2 = sX1X2_res->Id();

        // Now, test matrix...
        EXPECT_EQ(system.size(), 3);
        EXPECT_EQ(matrix_id, 2);
        ASSERT_TRUE(matrix.is_polynomial());
        EXPECT_EQ((matrix.SymbolMatrix(0, 0)), Polynomial(Monomial(sX, 1.0))); // 0.5 <X1> + 0.5<X2> = <X1> (after alias)
        EXPECT_EQ((matrix.SymbolMatrix(0, 1)), factory({Monomial(1, 0.5), Monomial{sX1X2, 0.5}}));

    }

}