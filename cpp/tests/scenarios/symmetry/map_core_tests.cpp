/**
 * map_core_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "scenarios/symmetrized/map_core.h"

#include "symbolic/symbol_table.h"

#include "sparse_utils.h"

#include <stdexcept>

namespace Moment::Tests {

    using namespace Moment::Symmetrized;
    namespace {
        std::shared_ptr<Algebraic::AlgebraicMatrixSystem> setup_ams2() {
            auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(Algebraic::NameTable{"a", "b"})
            );
            auto &ams = *amsPtr;
            ams.generate_dictionary(1);
            return amsPtr;
        }
        
        std::shared_ptr<Algebraic::AlgebraicMatrixSystem> setup_ams3() {
            auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(Algebraic::NameTable{"a", "b", "c"})
            );
            auto &ams = *amsPtr;
            ams.generate_dictionary(1);
            return amsPtr;
        }
    };

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow1_Dense) {
        auto amsPtr = setup_ams2();


        Eigen::MatrixXd m(3,3);
        m << 2.0, 2.0, 3.0,
                0.0, 0.0, 0.0,
                0.0, 0.0, 0.0;
        EXPECT_THROW((MapCore{amsPtr->Symbols(), m}), std::range_error);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow1_Sparse) {
        auto amsPtr = setup_ams2();

        Eigen::SparseMatrix<double> m = make_sparse(3, {2.0, 2.0, 3.0,
                                                        0.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});

        EXPECT_THROW((MapCore{amsPtr->Symbols(), m}), std::range_error);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow2_Dense) {
        auto amsPtr = setup_ams2();


        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                2.0, 0.0, 0.0,
                0.0, 0.0, 0.0;
        EXPECT_THROW((MapCore{amsPtr->Symbols(), m}), std::range_error);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow2_Sparse) {
        auto amsPtr = setup_ams2();

        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        2.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});

        EXPECT_THROW((MapCore{amsPtr->Symbols(), m}), std::range_error);
    }


    TEST(Scenarios_Symmetry_MapCore, ThreeConstants_Dense) {
        auto amsPtr = setup_ams2();

        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                0.0, 0.0, 0.0,
                0.0, 0.0, 0.0;

        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 2);
        EXPECT_EQ(core.constants[1], 2.0);
        EXPECT_EQ(core.constants[2], 3.0);
        EXPECT_EQ(core.core.rows(), 0);
        EXPECT_EQ(core.core.cols(), 0);
        EXPECT_EQ(core.nontrivial_rows.count(), 0);
        EXPECT_EQ(core.nontrivial_cols.count(), 0);
    }

    TEST(Scenarios_Symmetry_MapCore, ThreeConstants_Sparse) {
        auto amsPtr = setup_ams2();

        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        0.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});
        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 2);
        EXPECT_EQ(core.constants[1], 2.0);
        EXPECT_EQ(core.constants[2], 3.0);
        EXPECT_EQ(core.core.rows(), 0);
        EXPECT_EQ(core.core.cols(), 0);
        EXPECT_EQ(core.nontrivial_rows.count(), 0);
        EXPECT_EQ(core.nontrivial_cols.count(), 0);
    }


    TEST(Scenarios_Symmetry_MapCore, Full_Dense) {
        auto amsPtr = setup_ams2();

        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                0.0, 4.0, 5.0,
                0.0, 6.0, 7.0;

        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 0);
        ASSERT_EQ(core.core_offset.size(), 2);
        EXPECT_EQ(core.core_offset[0], 2.0);
        EXPECT_EQ(core.core_offset[1], 3.0);
        ASSERT_EQ(core.core.rows(), 2);
        ASSERT_EQ(core.core.cols(), 2);
        EXPECT_EQ(core.nontrivial_rows.count(), 2);
        EXPECT_EQ(core.nontrivial_cols.count(), 2);
        EXPECT_EQ(core.core(0,0), 4.0);
        EXPECT_EQ(core.core(0,1), 5.0);
        EXPECT_EQ(core.core(1,0), 6.0);
        EXPECT_EQ(core.core(1,1), 7.0);
    }

    TEST(Scenarios_Symmetry_MapCore, Full_Sparse) {
        auto amsPtr = setup_ams2();

        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        0.0, 4.0, 5.0,
                                                        0.0, 6.0, 7.0});

        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 0);
        ASSERT_EQ(core.core_offset.size(), 2);
        EXPECT_EQ(core.core_offset[0], 2.0);
        EXPECT_EQ(core.core_offset[1], 3.0);
        ASSERT_EQ(core.core.rows(), 2);
        ASSERT_EQ(core.core.cols(), 2);
        EXPECT_EQ(core.nontrivial_rows.count(), 2);
        EXPECT_EQ(core.nontrivial_cols.count(), 2);
        EXPECT_EQ(core.core(0,0), 4.0);
        EXPECT_EQ(core.core(0,1), 5.0);
        EXPECT_EQ(core.core(1,0), 6.0);
        EXPECT_EQ(core.core(1,1), 7.0);
    }

    TEST(Scenarios_Symmetry_MapCore, ClipCol_Dense) {
        auto amsPtr = setup_ams3();

        Eigen::MatrixXd m(4,4);
        m << 1.0, 2.0, 3.0, 4.0,
             0.0, 4.0, 5.0, 0.0,
             0.0, 6.0, 7.0, 0.0,
             0.0, 0.0, 0.0, 0.0;

        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 1);
        EXPECT_EQ(core.constants[3], 4.0);
        ASSERT_EQ(core.core_offset.size(), 2);
        EXPECT_EQ(core.core_offset[0], 2.0);
        EXPECT_EQ(core.core_offset[1], 3.0);
        ASSERT_EQ(core.core.rows(), 2);
        ASSERT_EQ(core.core.cols(), 2);
        EXPECT_EQ(core.nontrivial_rows.count(), 2);
        EXPECT_EQ(core.nontrivial_cols.count(), 2);
        EXPECT_EQ(core.core(0,0), 4.0);
        EXPECT_EQ(core.core(0,1), 5.0);
        EXPECT_EQ(core.core(1,0), 6.0);
        EXPECT_EQ(core.core(1,1), 7.0);
    }

    TEST(Scenarios_Symmetry_MapCore, ClipCol_Sparse) {
        auto amsPtr = setup_ams3();

        Eigen::SparseMatrix<double> m = make_sparse(4, {1.0, 2.0, 3.0, 4.0,
                                                        0.0, 4.0, 5.0, 0.0,
                                                        0.0, 6.0, 7.0, 0.0,
                                                        0.0, 0.0, 0.0, 0.0});

        MapCore core{amsPtr->Symbols(), m};
        ASSERT_EQ(core.constants.size(), 1);
        EXPECT_EQ(core.constants[3], 4.0);
        ASSERT_EQ(core.core_offset.size(), 2);
        EXPECT_EQ(core.core_offset[0], 2.0);
        EXPECT_EQ(core.core_offset[1], 3.0);
        ASSERT_EQ(core.core.rows(), 2);
        ASSERT_EQ(core.core.cols(), 2);
        EXPECT_EQ(core.nontrivial_rows.count(), 2);
        EXPECT_EQ(core.nontrivial_cols.count(), 2);
        EXPECT_EQ(core.core(0,0), 4.0);
        EXPECT_EQ(core.core(0,1), 5.0);
        EXPECT_EQ(core.core(1,0), 6.0);
        EXPECT_EQ(core.core(1,1), 7.0);
    }

}