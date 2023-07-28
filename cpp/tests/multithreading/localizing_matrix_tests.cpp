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
#include "matrix/operator_matrix/localizing_matrix.h"
#include "scenarios/context.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"


namespace Moment::Tests {
    using namespace Moment::Multithreading;

    TEST(Multithreading_LocalizingMatrix, Level1) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem system{std::make_unique<AlgebraicContext>(3)}; // 1 x y z
        const auto &context = system.AlgebraicContext();
        const oper_name_t x = 0, y = 1, z = 2;


        auto [id1, matLevel1] = system.LocalizingMatrix.create(LocalizingMatrixIndex{1, OperatorSequence({x}, context)},
                                                               Multithreading::MultiThreadPolicy::Always);
        ASSERT_EQ(matLevel1.Dimension(), 4);
        compare_lm_os_matrix(matLevel1, 4, {OperatorSequence({x}, context),
                                            OperatorSequence({x, x}, context),
                                            OperatorSequence({x, y}, context),
                                            OperatorSequence({x, z}, context),
                                            OperatorSequence({x, x}, context),
                                            OperatorSequence({x, x, x}, context),
                                            OperatorSequence({x, x, y}, context),
                                            OperatorSequence({x, x, z}, context),
                                            OperatorSequence({y, x}, context),
                                            OperatorSequence({y, x, x}, context),
                                            OperatorSequence({y, x, y}, context),
                                            OperatorSequence({y, x, z}, context),
                                            OperatorSequence({z, x}, context),
                                            OperatorSequence({z, x, x}, context),
                                            OperatorSequence({z, x, y}, context),
                                            OperatorSequence({z, x, z}, context)});
    }

    TEST(Multithreading_LocalizingMatrix, Level2) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem system{std::make_unique<AlgebraicContext>(3)}; // 1 x y z
        const auto& context = system.AlgebraicContext();
        auto [id2, matLevel2] = system.LocalizingMatrix.create(LocalizingMatrixIndex{2, OperatorSequence({0}, context)},
                                                               Multithreading::MultiThreadPolicy::Always);
        ASSERT_EQ(matLevel2.Dimension(), 13);
    }
}