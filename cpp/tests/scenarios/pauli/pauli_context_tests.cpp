/**
 * pauli_context_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_Context, Empty) {
        PauliContext context{0};
        EXPECT_EQ(context.size(), 0);
    }

    TEST(Scenarios_Pauli_Context, SigmaXYZ) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        // Sigma X1
        auto sigmaX1 = context.sigmaX(0);
        ASSERT_EQ(sigmaX1.size(), 1);
        EXPECT_EQ(sigmaX1[0], 0);
        EXPECT_EQ(sigmaX1.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaX1.hash(), context.hash({0}));

        // Sigma X2
        auto sigmaX2 = context.sigmaX(1);
        ASSERT_EQ(sigmaX2.size(), 1);
        EXPECT_EQ(sigmaX2[0], 3);
        EXPECT_EQ(sigmaX2.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaX2.hash(), context.hash({3}));

        // Sigma Y1
        auto sigmaY1 = context.sigmaY(0);
        ASSERT_EQ(sigmaY1.size(), 1);
        EXPECT_EQ(sigmaY1[0], 1);
        EXPECT_EQ(sigmaY1.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaY1.hash(), context.hash({1}));

        // Sigma Y2
        auto sigmaY2 = context.sigmaY(1);
        ASSERT_EQ(sigmaY2.size(), 1);
        EXPECT_EQ(sigmaY2[0], 4);
        EXPECT_EQ(sigmaY2.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaY2.hash(), context.hash({4}));

        // Sigma Z1
        auto sigmaZ1 = context.sigmaZ(0);
        ASSERT_EQ(sigmaZ1.size(), 1);
        EXPECT_EQ(sigmaZ1[0], 2);
        EXPECT_EQ(sigmaZ1.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaZ1.hash(), context.hash({2}));

        // Sigma Z2
        auto sigmaZ2 = context.sigmaZ(1);
        ASSERT_EQ(sigmaZ2.size(), 1);
        EXPECT_EQ(sigmaZ2[0], 5);
        EXPECT_EQ(sigmaZ2.get_sign(), SequenceSignType::Positive);
        EXPECT_EQ(sigmaZ2.hash(), context.hash({5}));
    }


    TEST(Scenarios_Pauli_Context, OperatorSequence_Single) {
        PauliContext context{2};

        EXPECT_EQ(OperatorSequence({0}, context), context.sigmaX(0));
        EXPECT_EQ(OperatorSequence({1}, context), context.sigmaY(0));
        EXPECT_EQ(OperatorSequence({2}, context), context.sigmaZ(0));
        EXPECT_EQ(OperatorSequence({3}, context), context.sigmaX(1));
        EXPECT_EQ(OperatorSequence({4}, context), context.sigmaY(1));
        EXPECT_EQ(OperatorSequence({5}, context), context.sigmaZ(1));
    }

    TEST(Scenarios_Pauli_Context, OperatorSequence_Joint) {
        PauliContext context{2};

        for (oper_name_t qubitA = 0; qubitA < 3; ++qubitA) {
            for (oper_name_t qubitB = 3; qubitB < 6; ++qubitB) {
                OperatorSequence pauliAB{{qubitA, qubitB}, context};
                ASSERT_EQ(pauliAB.size(), 2) << qubitA << "," << qubitB;
                EXPECT_EQ(pauliAB[0], qubitA);
                EXPECT_EQ(pauliAB[1], qubitB);
            }
        }
    }

    TEST(Scenarios_Pauli_Context, OperatorSequence_JointOutOfOrder) {
        PauliContext context{2};

        for (oper_name_t qubitA = 0; qubitA < 3; ++qubitA) {
            for (oper_name_t qubitB = 3; qubitB < 6; ++qubitB) {
                OperatorSequence pauliAB{{qubitB, qubitA}, context};
                ASSERT_EQ(pauliAB.size(), 2) << qubitB << "," << qubitA;
                EXPECT_EQ(pauliAB[0], qubitA);
                EXPECT_EQ(pauliAB[1], qubitB);
            }
        }
    }


    TEST(Scenarios_Pauli_Context, OperatorSequence_SingleByMult) {
        PauliContext context{1};
        ASSERT_EQ(context.qubit_size, 1);
        ASSERT_EQ(context.size(), 3);

        EXPECT_EQ(OperatorSequence({0, 0}, context), OperatorSequence::Identity(context));
        EXPECT_EQ(OperatorSequence({0, 1}, context), context.sigmaZ(0, SequenceSignType::Imaginary));
        EXPECT_EQ(OperatorSequence({0, 2}, context), context.sigmaY(0, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(OperatorSequence({1, 0}, context), context.sigmaZ(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(OperatorSequence({1, 1}, context), OperatorSequence::Identity(context));
        EXPECT_EQ(OperatorSequence({1, 2}, context), context.sigmaX(0, SequenceSignType::Imaginary));

        EXPECT_EQ(OperatorSequence({2, 0}, context), context.sigmaY(0, SequenceSignType::Imaginary));
        EXPECT_EQ(OperatorSequence({2, 1}, context), context.sigmaX(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(OperatorSequence({2, 2}, context), OperatorSequence::Identity(context));

        EXPECT_EQ(OperatorSequence({0, 0, 0}, context), context.sigmaX(0));
        EXPECT_EQ(OperatorSequence({1, 1, 0}, context), context.sigmaX(0));
        EXPECT_EQ(OperatorSequence({2, 2, 0}, context), context.sigmaX(0));

        EXPECT_EQ(OperatorSequence({0, 0, 1}, context), context.sigmaY(0));
        EXPECT_EQ(OperatorSequence({1, 1, 1}, context), context.sigmaY(0));
        EXPECT_EQ(OperatorSequence({2, 2, 1}, context), context.sigmaY(0));

        EXPECT_EQ(OperatorSequence({0, 0, 2}, context), context.sigmaZ(0));
        EXPECT_EQ(OperatorSequence({1, 1, 2}, context), context.sigmaZ(0));
        EXPECT_EQ(OperatorSequence({2, 2, 2}, context), context.sigmaZ(0));

        EXPECT_EQ(OperatorSequence({1, 0, 1}, context), context.sigmaX(0, SequenceSignType::Negative));
        EXPECT_EQ(OperatorSequence({2, 0, 2}, context), context.sigmaX(0, SequenceSignType::Negative));

        EXPECT_EQ(OperatorSequence({0, 1, 0}, context), context.sigmaY(0, SequenceSignType::Negative));
        EXPECT_EQ(OperatorSequence({2, 1, 2}, context), context.sigmaY(0, SequenceSignType::Negative));

        EXPECT_EQ(OperatorSequence({0, 2, 0}, context), context.sigmaZ(0, SequenceSignType::Negative));
        EXPECT_EQ(OperatorSequence({1, 2, 1}, context), context.sigmaZ(0, SequenceSignType::Negative));

        EXPECT_EQ(OperatorSequence({0, 1, 2}, context),
                  OperatorSequence::Identity(context, SequenceSignType::Imaginary));
        EXPECT_EQ(OperatorSequence({1, 2, 0}, context),
                  OperatorSequence::Identity(context, SequenceSignType::Imaginary));
        EXPECT_EQ(OperatorSequence({2, 0, 1}, context),
                  OperatorSequence::Identity(context, SequenceSignType::Imaginary));

        EXPECT_EQ(OperatorSequence({1, 0, 2}, context),
                  OperatorSequence::Identity(context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(OperatorSequence({2, 1, 0}, context),
                  OperatorSequence::Identity(context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(OperatorSequence({0, 2, 1}, context),
                  OperatorSequence::Identity(context, SequenceSignType::NegativeImaginary));
    }

    TEST(Scenarios_Pauli_Context, OperatorSequence_MultInStart) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        for (oper_name_t off_qubit = 3; off_qubit < 6; ++off_qubit) {
            EXPECT_EQ(OperatorSequence({0, 0, off_qubit}, context),
                      OperatorSequence({off_qubit}, context));
            EXPECT_EQ(OperatorSequence({0, 1, off_qubit}, context),
                      OperatorSequence({2, off_qubit}, context, SequenceSignType::Imaginary));
            EXPECT_EQ(OperatorSequence({0, 2, off_qubit}, context),
                      OperatorSequence({1, off_qubit}, context, SequenceSignType::NegativeImaginary));

            EXPECT_EQ(OperatorSequence({1, 0, off_qubit}, context),
                      OperatorSequence({2, off_qubit}, context, SequenceSignType::NegativeImaginary));
            EXPECT_EQ(OperatorSequence({1, 1, off_qubit}, context),
                      OperatorSequence({off_qubit}, context));
            EXPECT_EQ(OperatorSequence({1, 2, off_qubit}, context),
                      OperatorSequence({0, off_qubit}, context, SequenceSignType::Imaginary));

            EXPECT_EQ(OperatorSequence({2, 0, off_qubit}, context),
                      OperatorSequence({1, off_qubit}, context, SequenceSignType::Imaginary));
            EXPECT_EQ(OperatorSequence({2, 1, off_qubit}, context),
                      OperatorSequence({0, off_qubit}, context, SequenceSignType::NegativeImaginary));
            EXPECT_EQ(OperatorSequence({2, 2, off_qubit}, context),
                      OperatorSequence({off_qubit}, context));
        }
    }

    TEST(Scenarios_Pauli_Context, OperatorSequence_MultInEnd) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        for (oper_name_t off_qubit = 0; off_qubit < 3; ++off_qubit) {
            EXPECT_EQ(OperatorSequence({off_qubit, 3, 3}, context),
                      OperatorSequence({off_qubit}, context));
            EXPECT_EQ(OperatorSequence({off_qubit, 3, 4}, context),
                      OperatorSequence({off_qubit, 5}, context, SequenceSignType::Imaginary));
            EXPECT_EQ(OperatorSequence({off_qubit, 3, 5}, context),
                      OperatorSequence({off_qubit, 4}, context, SequenceSignType::NegativeImaginary));

            EXPECT_EQ(OperatorSequence({off_qubit, 4, 3}, context),
                      OperatorSequence({off_qubit, 5}, context, SequenceSignType::NegativeImaginary));
            EXPECT_EQ(OperatorSequence({off_qubit, 4, 4}, context),
                      OperatorSequence({off_qubit}, context));
            EXPECT_EQ(OperatorSequence({off_qubit, 4, 5}, context),
                      OperatorSequence({off_qubit, 3}, context, SequenceSignType::Imaginary));

            EXPECT_EQ(OperatorSequence({off_qubit, 5, 3}, context),
                      OperatorSequence({off_qubit, 4}, context, SequenceSignType::Imaginary));
            EXPECT_EQ(OperatorSequence({off_qubit, 5, 4}, context),
                      OperatorSequence({off_qubit, 3}, context, SequenceSignType::NegativeImaginary));
            EXPECT_EQ(OperatorSequence({off_qubit, 5, 5}, context),
                      OperatorSequence({off_qubit}, context));
        }
    }

    TEST(Scenarios_Pauli_Context, OperatorSequence_MultInMiddle) {
        PauliContext context{3};
        ASSERT_EQ(context.qubit_size, 3);
        ASSERT_EQ(context.size(), 9);

        for (oper_name_t off_qubitA = 0; off_qubitA < 3; ++off_qubitA) {
            for (oper_name_t off_qubitB = 6; off_qubitB < 9; ++off_qubitB) {
                EXPECT_EQ(OperatorSequence({off_qubitA, 3, 3, off_qubitB}, context),
                          OperatorSequence({off_qubitA, off_qubitB}, context));
                EXPECT_EQ(OperatorSequence({off_qubitA, 3, 4, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 5, off_qubitB}, context, SequenceSignType::Imaginary));
                EXPECT_EQ(OperatorSequence({off_qubitA, 3, 5, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 4, off_qubitB}, context, SequenceSignType::NegativeImaginary));

                EXPECT_EQ(OperatorSequence({off_qubitA, 4, 3, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 5, off_qubitB}, context, SequenceSignType::NegativeImaginary));
                EXPECT_EQ(OperatorSequence({off_qubitA, 4, 4, off_qubitB}, context),
                          OperatorSequence({off_qubitA, off_qubitB}, context));
                EXPECT_EQ(OperatorSequence({off_qubitA, 4, 5, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 3, off_qubitB}, context, SequenceSignType::Imaginary));

                EXPECT_EQ(OperatorSequence({off_qubitA, 5, 3, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 4, off_qubitB}, context, SequenceSignType::Imaginary));
                EXPECT_EQ(OperatorSequence({off_qubitA, 5, 4, off_qubitB}, context),
                          OperatorSequence({off_qubitA, 3, off_qubitB}, context, SequenceSignType::NegativeImaginary));
                EXPECT_EQ(OperatorSequence({off_qubitA, 5, 5, off_qubitB}, context),
                          OperatorSequence({off_qubitA, off_qubitB}, context));
            }
        }
    }

    TEST(Scenarios_Pauli_Context, Multiply_SingleQubit) {
        PauliContext context{1};
        ASSERT_EQ(context.qubit_size, 1);
        ASSERT_EQ(context.size(), 3);

        OperatorSequence x = context.sigmaX(0);
        OperatorSequence y = context.sigmaY(0);
        OperatorSequence z = context.sigmaZ(0);

        EXPECT_EQ(x * x, OperatorSequence::Identity(context));
        EXPECT_EQ(x * y, context.sigmaZ(0, SequenceSignType::Imaginary));
        EXPECT_EQ(x * z, context.sigmaY(0, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(y * x, context.sigmaZ(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(y * y, OperatorSequence::Identity(context));
        EXPECT_EQ(y * z, context.sigmaX(0, SequenceSignType::Imaginary));

        EXPECT_EQ(z * x, context.sigmaY(0, SequenceSignType::Imaginary));
        EXPECT_EQ(z * y, context.sigmaX(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(z * z, OperatorSequence::Identity(context));
    }

    TEST(Scenarios_Pauli_Context, Multiply_TwoQubits) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        OperatorSequence x0 = context.sigmaX(0);
        OperatorSequence y0 = context.sigmaY(0);
        OperatorSequence z0 = context.sigmaZ(0);

        OperatorSequence x1 = context.sigmaX(1);
        OperatorSequence y1 = context.sigmaY(1);
        OperatorSequence z1 = context.sigmaZ(1);

        // Test qubit 1
        EXPECT_EQ(x0 * x0, OperatorSequence::Identity(context));
        EXPECT_EQ(x0 * y0, context.sigmaZ(0, SequenceSignType::Imaginary));
        EXPECT_EQ(x0 * z0, context.sigmaY(0, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(y0 * x0, context.sigmaZ(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(y0 * y0, OperatorSequence::Identity(context));
        EXPECT_EQ(y0 * z0, context.sigmaX(0, SequenceSignType::Imaginary));

        EXPECT_EQ(z0 * x0, context.sigmaY(0, SequenceSignType::Imaginary));
        EXPECT_EQ(z0 * y0, context.sigmaX(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(z0 * z0, OperatorSequence::Identity(context));

        // Test qubit 2
        EXPECT_EQ(x1 * x1, OperatorSequence::Identity(context));
        EXPECT_EQ(x1 * y1, context.sigmaZ(1, SequenceSignType::Imaginary));
        EXPECT_EQ(x1 * z1, context.sigmaY(1, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(y1 * x1, context.sigmaZ(1, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(y1 * y1, OperatorSequence::Identity(context));
        EXPECT_EQ(y1 * z1, context.sigmaX(1, SequenceSignType::Imaginary));

        EXPECT_EQ(z1 * x1, context.sigmaY(1, SequenceSignType::Imaginary));
        EXPECT_EQ(z1 * y1, context.sigmaX(1, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(z1 * z1, OperatorSequence::Identity(context));

        // Test commutation
        for (oper_name_t qubitA = 0; qubitA < 3; ++qubitA) {
            for (oper_name_t qubitB = 3; qubitB < 6; ++qubitB) {
                OperatorSequence pauliA{{qubitA}, context};
                OperatorSequence pauliB{{qubitB}, context};
                OperatorSequence pauliAB{{qubitA, qubitB}, context};

                ASSERT_EQ(pauliA.size(), 1) << qubitA;
                ASSERT_EQ(pauliA[0], qubitA);
                ASSERT_EQ(pauliB.size(), 1) << qubitB;
                ASSERT_EQ(pauliB[0], qubitB);
                ASSERT_EQ(pauliAB.size(), 2) << qubitA << "," << qubitB;
                ASSERT_EQ(pauliAB[0], qubitA);
                ASSERT_EQ(pauliAB[1], qubitB);

                auto productAB = pauliA * pauliB;
                EXPECT_EQ(productAB, pauliAB) << qubitA << "," << qubitB;

                auto productBA = pauliB * pauliA;
                EXPECT_EQ(productBA, pauliAB) << qubitA << "," << qubitB;
            }
        }
    }

    TEST(Scenarios_Pauli_Context, Multiply_TwoQubitsByOne) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        OperatorSequence x0 = context.sigmaX(0);
        OperatorSequence y0 = context.sigmaY(0);
        OperatorSequence z0 = context.sigmaZ(0);

        OperatorSequence x0x1{{0, 3}, context};
        OperatorSequence y0x1{{1, 3}, context};
        OperatorSequence z0x1{{2, 3}, context};

        // 2 by 1
        EXPECT_EQ(x0x1 * x0, OperatorSequence({3}, context));
        EXPECT_EQ(x0x1 * y0, OperatorSequence({2, 3}, context, SequenceSignType::Imaginary));
        EXPECT_EQ(x0x1 * z0, OperatorSequence({1, 3}, context, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(y0x1 * x0, OperatorSequence({2, 3}, context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(y0x1 * y0, OperatorSequence({3}, context));
        EXPECT_EQ(y0x1 * z0, OperatorSequence({0, 3}, context, SequenceSignType::Imaginary));

        EXPECT_EQ(z0x1 * x0, OperatorSequence({1, 3}, context, SequenceSignType::Imaginary));
        EXPECT_EQ(z0x1 * y0, OperatorSequence({0, 3}, context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(z0x1 * z0, OperatorSequence({3}, context));

        // 1 by 2
        EXPECT_EQ(x0 * x0x1, OperatorSequence({3}, context));
        EXPECT_EQ(y0 * x0x1, OperatorSequence({2, 3}, context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(z0 * x0x1, OperatorSequence({1, 3}, context, SequenceSignType::Imaginary));

        EXPECT_EQ(x0 * y0x1, OperatorSequence({2, 3}, context, SequenceSignType::Imaginary));
        EXPECT_EQ(y0 * y0x1, OperatorSequence({3}, context));
        EXPECT_EQ(z0 * y0x1, OperatorSequence({0, 3}, context, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(x0 * z0x1, OperatorSequence({1, 3}, context, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(y0 * z0x1, OperatorSequence({0, 3}, context, SequenceSignType::Imaginary));
        EXPECT_EQ(z0 * z0x1, OperatorSequence({3}, context));
    }

    TEST(Scenarios_Pauli_Context, Conjugate_SingleQubit) {
        PauliContext context{1};
        ASSERT_EQ(context.qubit_size, 1);
        ASSERT_EQ(context.size(), 3);

        EXPECT_EQ(context.conjugate(context.sigmaX(0)), context.sigmaX(0));
        EXPECT_EQ(context.conjugate(context.sigmaY(0)), context.sigmaY(0));
        EXPECT_EQ(context.conjugate(context.sigmaZ(0)), context.sigmaZ(0));

        EXPECT_EQ(context.conjugate(context.sigmaX(0, SequenceSignType::Imaginary)),
                  context.sigmaX(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(context.conjugate(context.sigmaY(0, SequenceSignType::Imaginary)),
                  context.sigmaY(0, SequenceSignType::NegativeImaginary));
        EXPECT_EQ(context.conjugate(context.sigmaZ(0, SequenceSignType::Imaginary)),
                  context.sigmaZ(0, SequenceSignType::NegativeImaginary));

        EXPECT_EQ(context.conjugate(context.sigmaX(0, SequenceSignType::Negative)),
                  context.sigmaX(0, SequenceSignType::Negative));
        EXPECT_EQ(context.conjugate(context.sigmaY(0, SequenceSignType::Negative)),
                  context.sigmaY(0, SequenceSignType::Negative));
        EXPECT_EQ(context.conjugate(context.sigmaZ(0, SequenceSignType::Negative)),
                  context.sigmaZ(0, SequenceSignType::Negative));

        EXPECT_EQ(context.conjugate(context.sigmaX(0, SequenceSignType::NegativeImaginary)),
                  context.sigmaX(0, SequenceSignType::Imaginary));
        EXPECT_EQ(context.conjugate(context.sigmaY(0, SequenceSignType::NegativeImaginary)),
                  context.sigmaY(0, SequenceSignType::Imaginary));
        EXPECT_EQ(context.conjugate(context.sigmaZ(0, SequenceSignType::NegativeImaginary)),
                  context.sigmaZ(0, SequenceSignType::Imaginary));

    }

    TEST(Scenarios_Pauli_Context, Conjugate_TwoQubit) {
        PauliContext context{2};
        ASSERT_EQ(context.qubit_size, 2);
        ASSERT_EQ(context.size(), 6);

        for (oper_name_t qubitA = 0; qubitA < 3; ++qubitA) {
            for (oper_name_t qubitB = 3; qubitB < 6; ++qubitB) {
                const OperatorSequence positive{{qubitA, qubitB}, context};
                auto positive_conjugate = context.conjugate(positive);
                EXPECT_EQ(positive_conjugate, positive);

                const OperatorSequence imaginary{{qubitA, qubitB}, context, SequenceSignType::Imaginary};
                auto imaginary_conjugate = context.conjugate(imaginary);
                EXPECT_EQ(imaginary_conjugate,
                          OperatorSequence({qubitA, qubitB}, context, SequenceSignType::NegativeImaginary));
            }
        }
    }
}