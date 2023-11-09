/**
 * operator_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "compare_os_matrix.h"

#include "matrix/monomial_matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

namespace Moment::Tests {
    TEST(Matrix_OperatorMatrix, PreMultiply) {
        Pauli::PauliMatrixSystem pms{1};
        const auto& context = pms.pauliContext;

        auto I = OperatorSequence{context};
        auto i = OperatorSequence{{}, context, SequenceSignType::Imaginary};
        auto mi = OperatorSequence{{}, context, SequenceSignType::NegativeImaginary};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto mx = context.sigmaX(0, SequenceSignType::Negative);
        auto my = context.sigmaY(0, SequenceSignType::Negative);
        auto mz = context.sigmaZ(0, SequenceSignType::Negative);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        auto sZ = pms.pauliContext.sigmaZ(0);

        auto& mmRaw = pms.MomentMatrix(1);
        // Compare operator sequences
        compare_mm_os_matrix(mmRaw, 4, {I, x, y, z,
                                        x, I, iz, miy,
                                        y, miz, I, ix,
                                        z, iy, mix, I});


        ASSERT_TRUE(mmRaw.has_operator_matrix());
        auto& mm_ops = mmRaw.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);
        auto zMM = mm_ops.pre_multiply(z);
        compare_os_matrix("Z*MM", zMM, 4, {z, iy, mix, I,
                                           iy, z, i, mx,
                                           mix, mi, z, my,
                                           I, x, y, z});

    }
    TEST(Matrix_OperatorMatrix, PostMultiply) {
        Pauli::PauliMatrixSystem pms{1};
        const auto& context = pms.pauliContext;

        auto I = OperatorSequence{context};
        auto i = OperatorSequence{{}, context, SequenceSignType::Imaginary};
        auto mi = OperatorSequence{{}, context, SequenceSignType::NegativeImaginary};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto mx = context.sigmaX(0, SequenceSignType::Negative);
        auto my = context.sigmaY(0, SequenceSignType::Negative);
        auto mz = context.sigmaZ(0, SequenceSignType::Negative);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        auto& mm = pms.MomentMatrix(1);
        // Compare operator sequences
        compare_mm_os_matrix(mm, 4, {I, x, y, z,
                                     x, I, iz, miy,
                                     y, miz, I, ix,
                                     z, iy, mix, I});

        ASSERT_TRUE(mm.has_operator_matrix());
        auto& mm_ops = mm.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);

        auto MMz = mm_ops.post_multiply(z);
        compare_os_matrix("MM * z", MMz, 4, {z, miy, ix , I,
                                             miy, z, i, x,
                                             ix, mi, z, y,
                                             I, mx, my, z});

    }
}
