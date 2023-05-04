/**
 * group_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "../sparse_utils.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/symmetrized/group.h"

#include <Eigen/Sparse>

#include <initializer_list>
#include <vector>

using namespace Moment::Symmetrized;

namespace Moment::Tests {

    void test_group_unique(const std::vector<Eigen::SparseMatrix<double>>& group) {
        for (size_t i = 0; i < group.size(); ++i) {
            for (size_t j = i+1; j < group.size(); ++j) {
                EXPECT_FALSE(group[i].isApprox(group[j])) << "i = " << i << ", j = " << j;
            }
        }
    }

    TEST(Scenarios_Symmetry_Group, Dimino_ID) {

        //Eigen::SparseMatrix<double> gen_a(2);
        auto group = Group::dimino_generation(std::vector<Eigen::SparseMatrix<double>>{});

        ASSERT_EQ(group.size(), 1);
        EXPECT_TRUE(group[0].isApprox(sparse_id(1)));
    }

    TEST(Scenarios_Symmetry_Group, Dimino_Z2_2d) {

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(2, {0, 1, 1, 0}));

        auto group = Group::dimino_generation(generators);

        ASSERT_EQ(group.size(), 2);
        EXPECT_TRUE(group[0].isApprox(sparse_id(2)));
        EXPECT_TRUE(group[1].isApprox(generators[0]));

    }

    TEST(Scenarios_Symmetry_Group, Dimino_Z2_4d) {

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

    TEST(Scenarios_Symmetry_Group, Dimino_S3) {

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

    TEST(Scenarios_Symmetry_Group, Dimino_S4) {

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

    TEST(Scenarios_Symmetry_Group, Dimino_D8) {
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

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList1) {
        auto dc = Group::decompose_build_list(1);
        EXPECT_TRUE(dc.empty());

    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList2) {
        auto dc = Group::decompose_build_list(2);
        ASSERT_EQ(dc.size(), 1);
        EXPECT_EQ(dc[0], 2);
    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList3) {
        auto dc = Group::decompose_build_list(3);
        ASSERT_EQ(dc.size(), 2);
        EXPECT_EQ(dc[0], 3);
        EXPECT_EQ(dc[1], 2);
    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList4) {
        auto dc = Group::decompose_build_list(4);
        ASSERT_EQ(dc.size(), 2);
        EXPECT_EQ(dc[0], 4);
        EXPECT_EQ(dc[1], 2);
    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList7) {
        auto dc = Group::decompose_build_list(7);
        ASSERT_EQ(dc.size(), 4);
        EXPECT_EQ(dc[0], 7);
        EXPECT_EQ(dc[1], 4);
        EXPECT_EQ(dc[2], 3);
        EXPECT_EQ(dc[3], 2);
    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList10) {
        auto dc = Group::decompose_build_list(10);
        ASSERT_EQ(dc.size(), 4);
        EXPECT_EQ(dc[0], 10);
        EXPECT_EQ(dc[1], 8);
        EXPECT_EQ(dc[2], 4);
        EXPECT_EQ(dc[3], 2);
    }

    TEST(Scenarios_Symmetry_Group, DecomposeBuildList21) {
        auto dc = Group::decompose_build_list(21);
        ASSERT_EQ(dc.size(), 6);
        EXPECT_EQ(dc[0], 21);
        EXPECT_EQ(dc[1], 16);
        EXPECT_EQ(dc[2], 8);
        EXPECT_EQ(dc[3], 5);
        EXPECT_EQ(dc[4], 4);
        EXPECT_EQ(dc[5], 2);
    }



    TEST(Scenarios_Symmetry_Group, CreateRepresentation_CHSH_1to2) {

        // CHSH
        Locality::LocalityContext context{Locality::Party::MakeList(2, 2, 2)};

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

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        Group group{context, std::move(base_rep)};
        auto& rep1 = group.representation(1);
        ASSERT_EQ(rep1.size(), 16);
        EXPECT_EQ(rep1.word_length, 1);

        auto& rep2 = group.create_representation(2);
        auto& rep2_alias = group.representation(2);
        auto& rep2_alias2 = group.create_representation(2);
        ASSERT_NE(&rep1, &rep2);
        EXPECT_EQ(&rep2, &rep2_alias);
        EXPECT_EQ(&rep2, &rep2_alias2);

        EXPECT_EQ(rep2.size(), 16);
        EXPECT_EQ(rep2.word_length, 2);
        EXPECT_EQ(rep2.dimension, 13);
        for (const auto& mat : rep2) {
            EXPECT_EQ(mat.rows(), 13);
            EXPECT_EQ(mat.cols(), 13);
        }

    }

    TEST(Scenarios_Symmetry_Group, CreateRepresentation_Z2_1to10) {

        // CHSH
        Algebraic::AlgebraicContext context{2};

        // Dihedral-8 group <-> symmetries of CHSH inequality.
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));
        auto group_elems = Group::dimino_generation(generators);

        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        Group group{context, std::move(base_rep)};

        auto& rep1 = group.representation(1);
        ASSERT_EQ(rep1.size(), 2);
        EXPECT_EQ(rep1.word_length, 1);
        EXPECT_FALSE(rep1[0].isApprox(rep1[1]));
        EXPECT_TRUE(rep1[0].isApprox(rep1[1] * rep1[1]));

        auto& rep10 = group.create_representation(10);
        ASSERT_EQ(rep10.size(), 2);
        EXPECT_EQ(rep10.word_length, 10);
        EXPECT_FALSE(rep10[0].isApprox(rep10[1]));
        EXPECT_TRUE(rep10[0].isApprox(rep10[1] * rep10[1]));
    }
}