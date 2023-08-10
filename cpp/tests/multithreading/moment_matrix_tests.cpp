/**
 * moment_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "../matrix/compare_os_matrix.h"
#include "../matrix/compare_symbol_matrix.h"


#include "matrix_system/matrix_system.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "scenarios/context.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"


namespace Moment::Tests {
    using namespace Moment::Multithreading;

    TEST(Multithreading_MomentMatrix, Level1) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem system{std::make_unique<AlgebraicContext>(3)}; // 1 x y z
        const auto &context = system.AlgebraicContext();

        ASSERT_FALSE(context.can_make_unexpected_nonhermitian_matrices());
        auto [id1, matLevel1] = system.MomentMatrix.create(1, Multithreading::MultiThreadPolicy::Always);
        ASSERT_EQ(matLevel1.Dimension(), 4);


        const oper_name_t x = 0, y = 1, z = 2;

        compare_mm_os_matrix(matLevel1, 4, {OperatorSequence::Identity(context),
                                            OperatorSequence({x}, context),
                                            OperatorSequence({y}, context),
                                            OperatorSequence({z}, context),
                                            OperatorSequence({x}, context),
                                            OperatorSequence({x, x}, context),
                                            OperatorSequence({x, y}, context),
                                            OperatorSequence({x, z}, context),
                                            OperatorSequence({y}, context),
                                            OperatorSequence({y, x}, context),
                                            OperatorSequence({y, y}, context),
                                            OperatorSequence({y, z}, context),
                                            OperatorSequence({z}, context),
                                            OperatorSequence({z, x}, context),
                                            OperatorSequence({z, y}, context),
                                            OperatorSequence({z, z}, context)});
    }

    TEST(Multithreading_MomentMatrix, Level2) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem system{std::make_unique<AlgebraicContext>(3)}; // 1 x y z
        const auto& context = system.AlgebraicContext();
        ASSERT_FALSE(context.can_make_unexpected_nonhermitian_matrices());
        auto [id2, matLevel2] = system.MomentMatrix.create(2, Multithreading::MultiThreadPolicy::Always);
        ASSERT_EQ(matLevel2.Dimension(), 13);
    }
}