/**
 * collins_gisin_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/matrix/moment_matrix.h"
#include "operators/locality/locality_matrix_system.h"
#include "operators/locality/locality_context.h"
#include "operators/locality/collins_gisin.h"

namespace NPATK::Tests {

    namespace {

    }


    TEST(Operators_Locality_CollinsGisin, Empty) {


    }

    TEST(Operators_Locality_CollinsGisin, CHSH) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        const auto& mm = system.create_moment_matrix(1);

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 2);
        ASSERT_EQ(bob.size(), 2);

        const auto& a0 = alice[0];
        const auto& a1 = alice[1];
        const auto& b0 = bob[0];
        const auto& b1 = bob[1];

        const auto& cgi = system.CollinsGisin();
        ASSERT_EQ(cgi.Dimensions.size(), 2);
        ASSERT_EQ(cgi.Dimensions[0], 3);
        ASSERT_EQ(cgi.Dimensions[1], 3);

        const OperatorSequence os_id{{}, context};
        const OperatorSequence os_a0{{a0}, context};
        const OperatorSequence os_a1{{a1}, context};
        const OperatorSequence os_b0{{b0}, context};
        const OperatorSequence os_b1{{b1}, context};
        const OperatorSequence os_a0b0{{a0, b0}, context};
        const OperatorSequence os_a0b1{{a0, b1}, context};
        const OperatorSequence os_a1b0{{a1, b0}, context};
        const OperatorSequence os_a1b1{{a1, b1}, context};

        const auto& s_id = system.Symbols().where(os_id);
        const auto& s_a0 = system.Symbols().where(os_a0);
        const auto& s_a1 = system.Symbols().where(os_a1);
        const auto& s_b0 = system.Symbols().where(os_b0);
        const auto& s_b1 = system.Symbols().where(os_b1);
        const auto& s_a0b0 = system.Symbols().where(os_a0b0);
        const auto& s_a0b1 = system.Symbols().where(os_a0b1);
        const auto& s_a1b0 = system.Symbols().where(os_a1b0);
        const auto& s_a1b1 = system.Symbols().where(os_a1b1);

        EXPECT_EQ(cgi.index_to_sequence({0, 0}), os_id);
        EXPECT_EQ(cgi.index_to_sequence({1, 0}), os_a0);
        EXPECT_EQ(cgi.index_to_sequence({2, 0}), os_a1);
        EXPECT_EQ(cgi.index_to_sequence({0, 1}), os_b0);
        EXPECT_EQ(cgi.index_to_sequence({1, 1}), os_a0b0);
        EXPECT_EQ(cgi.index_to_sequence({2, 1}), os_a1b0);
        EXPECT_EQ(cgi.index_to_sequence({0, 2}), os_b1);
        EXPECT_EQ(cgi.index_to_sequence({1, 2}), os_a0b1);
        EXPECT_EQ(cgi.index_to_sequence({2, 2}), os_a1b1);

        ASSERT_EQ(cgi.Symbols().size(), 9);
        ASSERT_EQ(cgi.Sequences().size(), 9);
        ASSERT_EQ(cgi.RealIndices().size(), 9);

        EXPECT_EQ(cgi.index_to_offset({0, 0}), 0);
        EXPECT_EQ(cgi.Symbols()[0], s_id->Id());
        EXPECT_EQ(cgi.Sequences()[0], os_id);
        EXPECT_EQ(cgi.RealIndices()[0], s_id->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({1, 0}), 1);
        EXPECT_EQ(cgi.Symbols()[1], s_a0->Id());
        EXPECT_EQ(cgi.Sequences()[1], os_a0);
        EXPECT_EQ(cgi.RealIndices()[1], s_a0->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({2, 0}), 2);
        EXPECT_EQ(cgi.Symbols()[2], s_a1->Id());
        EXPECT_EQ(cgi.Sequences()[2], os_a1);
        EXPECT_EQ(cgi.RealIndices()[2], s_a1->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({0, 1}), 3);
        EXPECT_EQ(cgi.Symbols()[3], s_b0->Id());
        EXPECT_EQ(cgi.Sequences()[3], os_b0);
        EXPECT_EQ(cgi.RealIndices()[3], s_b0->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({1, 1}), 4);
        EXPECT_EQ(cgi.Symbols()[4], s_a0b0->Id());
        EXPECT_EQ(cgi.Sequences()[4], os_a0b0);
        EXPECT_EQ(cgi.RealIndices()[4], s_a0b0->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({2, 1}), 5);
        EXPECT_EQ(cgi.Symbols()[5], s_a1b0->Id());
        EXPECT_EQ(cgi.Sequences()[5], os_a1b0);
        EXPECT_EQ(cgi.RealIndices()[5], s_a1b0->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({0, 2}), 6);
        EXPECT_EQ(cgi.Symbols()[6], s_b1->Id());
        EXPECT_EQ(cgi.Sequences()[6], os_b1);
        EXPECT_EQ(cgi.RealIndices()[6], s_b1->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({1, 2}), 7);
        EXPECT_EQ(cgi.Symbols()[7], s_a0b1->Id());
        EXPECT_EQ(cgi.Sequences()[7], os_a0b1);
        EXPECT_EQ(cgi.RealIndices()[7], s_a0b1->basis_key().first);

        EXPECT_EQ(cgi.index_to_offset({2, 2}), 8);
        EXPECT_EQ(cgi.Symbols()[8], s_a1b1->Id());
        EXPECT_EQ(cgi.Sequences()[8], os_a1b1);
        EXPECT_EQ(cgi.RealIndices()[8], s_a1b1->basis_key().first);

    }

    TEST(Operators_Locality_CollinsGisin, BadIndices) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& mm = system.create_moment_matrix(1);
        const auto& cgi = system.CollinsGisin();

        std::vector<size_t> biA = {1};
        EXPECT_THROW(cgi.validate_index(biA), errors::BadCGError);

        std::vector<size_t> biB = {1, 1, 1};
        EXPECT_THROW(cgi.validate_index(biA), errors::BadCGError);

        std::vector<size_t> biC = {3, 1};
        EXPECT_THROW(cgi.validate_index(biA), errors::BadCGError);

        std::vector<size_t> biD = {1, 3};
        EXPECT_THROW(cgi.validate_index(biA), errors::BadCGError);
    }

}