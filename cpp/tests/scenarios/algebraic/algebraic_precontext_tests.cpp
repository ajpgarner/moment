/**
 * algebraic_precontext_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix/operator_sequence_generator.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "scenarios/operator_sequence.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Scenarios_Algebraic_AlgebraicPrecontext, Empty) {
        AlgebraicPrecontext apc{0};
        EXPECT_EQ(apc.num_operators, 0);
    }


    TEST(Scenarios_Algebraic_AlgebraicPrecontext, Conjugate_HermitianOps) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        EXPECT_EQ(apc.num_operators, 2);
        EXPECT_EQ(apc.raw_operators, 2);
        EXPECT_EQ(apc.conj_mode, AlgebraicPrecontext::ConjugateMode::SelfAdjoint);
        EXPECT_TRUE(apc.self_adjoint());

        sequence_storage_t ss{0, 0, 1};

        auto ss_conj = apc.conjugate(ss);
        ASSERT_EQ(ss_conj.size(), 3);
        EXPECT_EQ(ss_conj[0], 1);
        EXPECT_EQ(ss_conj[1], 0);
        EXPECT_EQ(ss_conj[2], 0);
    }

    TEST(Scenarios_Algebraic_AlgebraicPrecontext, Conjugate_NonHBunched) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        EXPECT_EQ(apc.num_operators, 4);
        EXPECT_EQ(apc.raw_operators, 2);
        EXPECT_EQ(apc.conj_mode, AlgebraicPrecontext::ConjugateMode::Bunched);
        EXPECT_FALSE(apc.self_adjoint());

        sequence_storage_t ss{0, 0, 1, 2};

        auto ss_conj = apc.conjugate(ss);
        ASSERT_EQ(ss_conj.size(), 4);
        EXPECT_EQ(ss_conj[0], 0);
        EXPECT_EQ(ss_conj[1], 3);
        EXPECT_EQ(ss_conj[2], 2);
        EXPECT_EQ(ss_conj[3], 2);
    }

    TEST(Scenarios_Algebraic_AlgebraicPrecontext, Conjugate_NonHInterleaved) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Interleaved};
        EXPECT_EQ(apc.num_operators, 4);
        EXPECT_EQ(apc.raw_operators, 2);
        EXPECT_EQ(apc.conj_mode, AlgebraicPrecontext::ConjugateMode::Interleaved);
        EXPECT_FALSE(apc.self_adjoint());

        sequence_storage_t ss{0, 0, 2, 1};

        auto ss_conj = apc.conjugate(ss);
        ASSERT_EQ(ss_conj.size(), 4);
        EXPECT_EQ(ss_conj[0], 0);
        EXPECT_EQ(ss_conj[1], 3);
        EXPECT_EQ(ss_conj[2], 1);
        EXPECT_EQ(ss_conj[3], 1);
    }
}