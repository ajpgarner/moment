/**
 * lu_mcp_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "scenarios/symmetrized/map_core.h"
#include "scenarios/symmetrized/lu_map_core_processor.h"

#include "symbolic/symbol_table.h"

#include "sparse_utils.h"

#include <stdexcept>

namespace Moment::Tests {

    using namespace Moment::Symmetrized;
    namespace {
        std::shared_ptr<Algebraic::AlgebraicMatrixSystem> setup_ams(size_t op_count) {
            auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(op_count)
            );
            auto &ams = *amsPtr;
            ams.generate_dictionary(1);
            return amsPtr;
        }
    }


    TEST(Scenarios_Symmetry_luMCP, Trivial) {

        auto amsPtr = setup_ams(2);
        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        0.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});
        MapCore core{amsPtr->Symbols(), m};

        LUMapCoreProcessor processor{};
        auto solution = core.accept(processor);
        EXPECT_TRUE(solution.trivial_solution);
        EXPECT_EQ(solution.output_symbols, 0);

        const auto& x_to_y = solution.map;
        const auto& y_to_x = solution.inv_map;
        EXPECT_EQ(x_to_y.rows(), 0);
        EXPECT_EQ(x_to_y.cols(), 0);
        EXPECT_EQ(y_to_x.rows(), 0);
        EXPECT_EQ(y_to_x.cols(), 0);

    }

    TEST(Scenarios_Symmetry_luMCP, RankReducingMap) {

        auto amsPtr = setup_ams(2);
        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 0.0, 0.0,
                                                        0.0, 1.0, 1.0,
                                                        0.0, 1.0, 1.0});
        MapCore core{amsPtr->Symbols(), m};

        LUMapCoreProcessor processor{};
        auto solution = core.accept(processor);
        EXPECT_FALSE(solution.trivial_solution);
        EXPECT_EQ(solution.output_symbols, 1);

        const auto& x_to_y = solution.map;
        const auto& y_to_x = solution.inv_map;
        ASSERT_EQ(x_to_y.rows(), 2);
        ASSERT_EQ(x_to_y.cols(), 1);
        ASSERT_EQ(y_to_x.rows(), 1);
        ASSERT_EQ(y_to_x.cols(), 2);

        EXPECT_DOUBLE_EQ(x_to_y(0,0), 1.0);
        EXPECT_DOUBLE_EQ(x_to_y(1,0), 1.0);
        EXPECT_DOUBLE_EQ(y_to_x(0,0), 1.0);
        EXPECT_DOUBLE_EQ(y_to_x(0,1), 1.0);
    }
}