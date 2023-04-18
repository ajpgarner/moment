/**
 * map_core_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/symmetrized/map_core.h"
#include "scenarios/symmetrized/symmetrized_errors.h"

#include "symbolic/symbol_table.h"

#include "utilities/dynamic_bitset.h"

#include "sparse_utils.h"

#include <stdexcept>

namespace Moment::Tests {

    using namespace Moment::Symmetrized;

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow1_Dense) {
        Eigen::MatrixXd m(3,3);
        m << 2.0, 2.0, 3.0,
                0.0, 0.0, 0.0,
                0.0, 0.0, 0.0;
        EXPECT_THROW((MapCore{DynamicBitset<size_t>(3,false), m}), errors::bad_map);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow1_Sparse) {
        Eigen::SparseMatrix<double> m = make_sparse(3, {2.0, 2.0, 3.0,
                                                        0.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});

        EXPECT_THROW((MapCore{DynamicBitset<size_t>(3,false), m}), errors::bad_map);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow2_Dense) {
        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                2.0, 0.0, 0.0,
                0.0, 0.0, 0.0;
        EXPECT_THROW((MapCore{DynamicBitset<size_t>(3,false), m}), errors::bad_map);
    }

    TEST(Scenarios_Symmetry_MapCore, BadFirstRow2_Sparse) {
        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        2.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});

        EXPECT_THROW((MapCore{DynamicBitset<size_t>(3,false), m}), errors::bad_map);
    }


    TEST(Scenarios_Symmetry_MapCore, ThreeConstants_Dense) {
        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                0.0, 0.0, 0.0,
                0.0, 0.0, 0.0;

        MapCore core{DynamicBitset<size_t>(3,false), m};
        ASSERT_EQ(core.constants.size(), 2);
        EXPECT_EQ(core.constants[1], 2.0);
        EXPECT_EQ(core.constants[2], 3.0);
        EXPECT_EQ(core.core.rows(), 0);
        EXPECT_EQ(core.core.cols(), 0);
        EXPECT_EQ(core.nontrivial_rows.count(), 0);
        EXPECT_EQ(core.nontrivial_cols.count(), 0);
    }

    TEST(Scenarios_Symmetry_MapCore, ThreeConstants_Sparse) {
        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        0.0, 0.0, 0.0,
                                                        0.0, 0.0, 0.0});
        MapCore core{DynamicBitset<size_t>(3,false), m};
        ASSERT_EQ(core.constants.size(), 2);
        EXPECT_EQ(core.constants[1], 2.0);
        EXPECT_EQ(core.constants[2], 3.0);
        EXPECT_EQ(core.core.rows(), 0);
        EXPECT_EQ(core.core.cols(), 0);
        EXPECT_EQ(core.nontrivial_rows.count(), 0);
        EXPECT_EQ(core.nontrivial_cols.count(), 0);
    }


    TEST(Scenarios_Symmetry_MapCore, Full_Dense) {
        Eigen::MatrixXd m(3,3);
        m << 1.0, 2.0, 3.0,
                0.0, 4.0, 5.0,
                0.0, 6.0, 7.0;

        MapCore core{DynamicBitset<size_t>(3,false), m};
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
        Eigen::SparseMatrix<double> m = make_sparse(3, {1.0, 2.0, 3.0,
                                                        0.0, 4.0, 5.0,
                                                        0.0, 6.0, 7.0});

        MapCore core{DynamicBitset<size_t>(3,false), m};
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
        Eigen::MatrixXd m(4,4);
        m << 1.0, 2.0, 3.0, 4.0,
             0.0, 4.0, 5.0, 0.0,
             0.0, 6.0, 7.0, 0.0,
             0.0, 0.0, 0.0, 0.0;

        MapCore core{DynamicBitset<size_t>(4,false), m};
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
        Eigen::SparseMatrix<double> m = make_sparse(4, {1.0, 2.0, 3.0, 4.0,
                                                        0.0, 4.0, 5.0, 0.0,
                                                        0.0, 6.0, 7.0, 0.0,
                                                        0.0, 0.0, 0.0, 0.0});

        MapCore core{DynamicBitset<size_t>(4,false), m};
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