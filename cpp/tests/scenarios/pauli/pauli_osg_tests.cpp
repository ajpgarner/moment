/**
 * pauli_osg_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_osg.h"

namespace Moment::Tests {
    using namespace Moment::Pauli;

    class OSGTester {
    public:
        const OperatorSequenceGenerator& osg;
        const Context& context;
        std::vector<OperatorSequence>::const_iterator iter;
        const std::vector<OperatorSequence>::const_iterator iter_end;

        explicit OSGTester(const OperatorSequenceGenerator& osg)
            : osg{osg}, context{osg.context}, iter{osg.begin()}, iter_end{osg.end()} {

        }

        void check_and_advance(const OperatorSequence& reference, const std::string& ctx = "") {
            ASSERT_NE(iter, iter_end) << ctx << "Reference sequence: " << reference;
            EXPECT_EQ(*iter, reference) << ctx;
            ++iter;
        }

        void test_pauli_single(const size_t party) {
            const auto& pauli_context = dynamic_cast<const Pauli::PauliContext&>(context);
            check_and_advance(pauli_context.sigmaX(party));
            check_and_advance(pauli_context.sigmaY(party));
            check_and_advance(pauli_context.sigmaZ(party));
        }

        void test_pauli_pairs(size_t party_A, size_t party_B) {
              std::string added_context = std::string("Qubit ") + std::to_string(party_A)
                                        + " with " + std::to_string(party_B) + "\n";

            const auto& pauli_context = dynamic_cast<const Pauli::PauliContext&>(context);
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaX(party_B), added_context + " XX");
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaY(party_B), added_context + " XY");
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaZ(party_B), added_context + " XZ");
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaX(party_B), added_context + " YX");
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaY(party_B), added_context + " YY");
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaZ(party_B), added_context + " YZ");
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaX(party_B), added_context + " ZX");
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaY(party_B), added_context + " ZY");
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaZ(party_B), added_context + " ZZ");
        }

        void expected_finished() {
            EXPECT_EQ(iter, iter_end);
        }
    };

    TEST(Scenarios_Pauli_OSG, OneQubit_LevelZero) {
        PauliContext context{1};
        ASSERT_EQ(context.size(), 3);

        PauliSequenceGenerator psg{context, 0};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 1);
        auto psg_iter = psg.begin();
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, OperatorSequence(context));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }

    TEST(Scenarios_Pauli_OSG, OneQubit_LevelOne) {
        PauliContext context{1};
        ASSERT_EQ(context.size(), 3);

        PauliSequenceGenerator psg{context, 1};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 4);

        auto psg_iter = psg.begin();
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, OperatorSequence(context));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(0));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(0));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(0));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }

    TEST(Scenarios_Pauli_OSG, TwoQubits_LevelOne) {
        PauliContext context{2};
        ASSERT_EQ(context.size(), 6);

        PauliSequenceGenerator psg{context, 1};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 7);

        OSGTester tester{psg};
        tester.check_and_advance(OperatorSequence(context));
        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, TwoQubits_LevelTwo) {
        PauliContext context{2};
        ASSERT_EQ(context.size(), 6);

        PauliSequenceGenerator psg{context, 2};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 16);
        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);

        tester.test_pauli_pairs(0, 1);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, ThreeQubits_NearestNeighbours) {
        PauliContext context{3, WrapType::None};
        ASSERT_EQ(context.size(), 9);
        ASSERT_EQ(context.wrap, WrapType::None);

        PauliSequenceGenerator psg{context, 2, 1};
        
        EXPECT_TRUE(psg.limited());
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 28); // L0: 1, L1: 9; L2: 18

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(1, 2);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, ThreeQubits_NearestNeighboursWrapped) {
        PauliContext context{3, WrapType::Wrap};
        ASSERT_EQ(context.size(), 9);
        ASSERT_EQ(context.wrap, WrapType::Wrap);

        PauliSequenceGenerator psg{context, 2, 1};
        EXPECT_TRUE(psg.limited());
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 37); // L0: 1, L1: 9; L2: 27

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(2, 0);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, FiveQubits_NextNearestNeighbours) {
        PauliContext context{5, WrapType::None};
        ASSERT_EQ(context.size(), 15);
        ASSERT_EQ(context.wrap, WrapType::None);

        PauliSequenceGenerator psg{context, 2, 2};
        EXPECT_TRUE(psg.limited());
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 2);
        EXPECT_EQ(psg.size(), 79); // L0: 1, L1: 15; L2: 63

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);
        tester.test_pauli_single(3);
        tester.test_pauli_single(4);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 2);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 3);
        tester.test_pauli_pairs(2, 3);
        tester.test_pauli_pairs(2, 4);
        tester.test_pauli_pairs(3, 4);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, FiveQubits_NextNearestNeighboursWrapped) {
        PauliContext context{5, WrapType::Wrap};
        ASSERT_EQ(context.size(), 15);
        ASSERT_EQ(context.wrap, WrapType::Wrap);

        PauliSequenceGenerator psg{context, 2, 2};
        EXPECT_TRUE(psg.limited());

        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 2);
        EXPECT_EQ(psg.size(), 106); // L0: 1, L1: 15; L2: 90

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);
        tester.test_pauli_single(3);
        tester.test_pauli_single(4);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 2);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 3);
        tester.test_pauli_pairs(2, 3);
        tester.test_pauli_pairs(2, 4);
        tester.test_pauli_pairs(3, 4);
        tester.test_pauli_pairs(3, 0);
        tester.test_pauli_pairs(4, 0);
        tester.test_pauli_pairs(4, 1);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, NineQubits_LatticeUnwrapped) {
        PauliContext context{3, 3, WrapType::None, SymmetryType::None};
        ASSERT_EQ(context.size(), 27);
        ASSERT_EQ(context.wrap, WrapType::None);
        ASSERT_EQ(context.qubit_size, 9);
        ASSERT_EQ(context.row_width, 3);
        ASSERT_EQ(context.col_height, 3);
        ASSERT_TRUE(context.is_lattice());

        PauliSequenceGenerator psg{context, 2, 1};
        EXPECT_TRUE(psg.limited());

        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 136); // L0: 1, L1: 27; L2: 108

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        for (size_t qubit = 0; qubit < 9; ++qubit) {
            tester.test_pauli_single(qubit);
        }

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 3);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 4);
        tester.test_pauli_pairs(2, 5);
        tester.test_pauli_pairs(3, 4);
        tester.test_pauli_pairs(3, 6);
        tester.test_pauli_pairs(4, 5);
        tester.test_pauli_pairs(4, 7);
        tester.test_pauli_pairs(5, 8);
        tester.test_pauli_pairs(6, 7);
        tester.test_pauli_pairs(7, 8);
    }

    TEST(Scenarios_Pauli_OSG, SixQubits_LatticeWrapped) {
        PauliContext context{3, 2, WrapType::Wrap, SymmetryType::None}; // 3x2 lattice
        ASSERT_EQ(context.wrap, WrapType::Wrap);
        ASSERT_EQ(context.size(), 18);
        ASSERT_EQ(context.qubit_size, 6);
        ASSERT_EQ(context.col_height, 3);
        ASSERT_EQ(context.row_width, 2);
        ASSERT_TRUE(context.is_lattice());

        PauliSequenceGenerator psg{context, 2, 1};
        EXPECT_TRUE(psg.limited());
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 100); // L0: 1, L1: 18; L2: 81

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        for (size_t qubit = 0; qubit < 6; ++qubit) {
            tester.test_pauli_single(qubit);
        }

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 3);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 4);
        tester.test_pauli_pairs(2, 0);
        tester.test_pauli_pairs(2, 5);

        tester.test_pauli_pairs(3, 4);
        tester.test_pauli_pairs(4, 5);
        tester.test_pauli_pairs(5, 3);

    }

    TEST(Scenarios_Pauli_OSG, NineQubits_LatticeWrapped) {
        PauliContext context{3, 3, WrapType::Wrap, SymmetryType::None};
        ASSERT_EQ(context.wrap, WrapType::Wrap);
        ASSERT_EQ(context.size(), 27);
        ASSERT_EQ(context.qubit_size, 9);
        ASSERT_EQ(context.row_width, 3);
        ASSERT_EQ(context.col_height, 3);
        ASSERT_TRUE(context.is_lattice());

        PauliSequenceGenerator psg{context, 2, 1};
        EXPECT_TRUE(psg.limited());
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 190); // L0: 1, L1: 27; L2: 162

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        for (size_t qubit = 0; qubit < 9; ++qubit) {
            tester.test_pauli_single(qubit);
        }

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 3);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 4);
        tester.test_pauli_pairs(2, 0);
        tester.test_pauli_pairs(2, 5);

        tester.test_pauli_pairs(3, 4);
        tester.test_pauli_pairs(3, 6);
        tester.test_pauli_pairs(4, 5);
        tester.test_pauli_pairs(4, 7);
        tester.test_pauli_pairs(5, 3);
        tester.test_pauli_pairs(5, 8);

        tester.test_pauli_pairs(6, 7);
        tester.test_pauli_pairs(6, 0);
        tester.test_pauli_pairs(7, 8);
        tester.test_pauli_pairs(7, 1);
        tester.test_pauli_pairs(8, 6);
        tester.test_pauli_pairs(8, 2);
    }


    TEST(Scenarios_Pauli_OSG, SixteenQubitLattice_WrappedSymmetric) {
        PauliContext context{4, 4, WrapType::Wrap, SymmetryType::Translational};
        ASSERT_EQ(context.wrap, WrapType::Wrap);
        ASSERT_EQ(context.size(), 48);
        ASSERT_EQ(context.qubit_size, 16);
        ASSERT_EQ(context.row_width, 4);
        ASSERT_EQ(context.col_height, 4);
        ASSERT_TRUE(context.is_lattice());

        // Word-3 NN generator
        PauliSequenceGenerator psg{context, 3, 1};
        ASSERT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        ASSERT_EQ(psg.nearest_neighbour_index.moment_matrix_level, 3);

        EXPECT_EQ(psg.size(), 2929); // L0: 1, L1: 48; L2: 288, L3: 2592
    }


    TEST(Scenarios_Pauli_OSG, SixteenQubitLattice_Thermodynamic) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        ASSERT_EQ(context.wrap, WrapType::None);
        ASSERT_EQ(context.translational_symmetry, SymmetryType::Translational);
        ASSERT_EQ(context.size(), 48);
        ASSERT_EQ(context.qubit_size, 16);
        ASSERT_EQ(context.row_width, 4);
        ASSERT_EQ(context.col_height, 4);
        ASSERT_TRUE(context.is_lattice());

        // Word-3 NN generator
        PauliSequenceGenerator psg{context, 3, 1};
        ASSERT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        ASSERT_EQ(psg.nearest_neighbour_index.moment_matrix_level, 3);

        EXPECT_EQ(psg.size(), 1669); // L0: 1, L1: 48; L2: 216, L3: 1404
    }

}