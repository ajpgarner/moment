/**
 * group_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "sparse_utils.h"

#include "symmetry/group.h"

#include <Eigen/Sparse>

#include <initializer_list>
#include <vector>

namespace Moment::Tests {

    void test_group_unique(const std::vector<Eigen::SparseMatrix<double>>& group) {
        for (size_t i = 0; i < group.size(); ++i) {
            for (size_t j = i+1; j < group.size(); ++j) {
                EXPECT_FALSE(group[i].isApprox(group[j])) << "i = " << i << ", j = " << j;
            }
        }
    }



    TEST(Symmetry_Group, Dimino_ID) {

        //Eigen::SparseMatrix<double> gen_a(2);
        auto group = Group::dimino_generation(std::vector<Eigen::SparseMatrix<double>>{});

        ASSERT_EQ(group.size(), 1);
        EXPECT_TRUE(group[0].isApprox(sparse_id(1)));
    }


    TEST(Symmetry_Group, Dimino_Z2_2d) {

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(2, {0, 1, 1, 0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 2);
        EXPECT_TRUE(group[0].isApprox(sparse_id(2)));
        EXPECT_TRUE(group[1].isApprox(generators[0]));

    }

    TEST(Symmetry_Group, Dimino_Z2_4d) {

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(4, {0, 1, 0, 0,
                                                1, 0, 0, 0,
                                                0, 0, 0, 1,
                                                0, 0, 1, 0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 2);
        EXPECT_TRUE(group[0].isApprox(sparse_id(4)));
        EXPECT_TRUE(group[1].isApprox(generators[0]));
    }

    TEST(Symmetry_Group, Dimino_S3) {

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(3, {0, 1, 0,
                                                1, 0, 0,
                                                0, 0, 1}));
        generators.emplace_back(make_sparse(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 6);
        EXPECT_TRUE(group[0].isApprox(sparse_id(3)));
        test_group_unique(group);
    }

    TEST(Symmetry_Group, Dimino_S4) {

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(4, {0, 1, 0, 0,
                                                1, 0, 0, 0,
                                                0, 0, 1, 0,
                                                0, 0, 0, 1}));
        generators.emplace_back(make_sparse(4, {1, 0, 0, 0,
                                                0, 0, 1, 0,
                                                0, 1, 0, 0,
                                                0, 0, 0, 1}));
        generators.emplace_back(make_sparse(4, {1, 0, 0, 0,
                                                0, 1, 0, 0,
                                                0, 0, 0, 1,
                                                0, 0, 1, 0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 24);
        EXPECT_TRUE(group[0].isApprox(sparse_id(4)));
        test_group_unique(group);
    }

    TEST(Symmetry_Group, Dimino_D8) {
        // Dihedral-8 group <-> symmetries of CHSH inequality.
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(5, {1, 0, 0, 0, 0,
                                                1, 0, 0, 0,-1,
                                                0, 0, 0, 1, 0,
                                                0, 1, 0, 0 ,0,
                                                0, 0, 1, 0 ,0}));
        generators.emplace_back(make_sparse(5, {1, 0, 0, 0, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 0, 1, 0,
                                                0, 0, 1, 0 ,0,
                                                0, 1, 0, 0 ,0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 16);
        EXPECT_TRUE(group[0].isApprox(sparse_id(5)));
        test_group_unique(group);
    }
}