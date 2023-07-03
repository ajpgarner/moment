/**
 * collins_gisin_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "scenarios/collins_gisin_iterator.h"

#include "scenarios/locality/locality_collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"


namespace Moment::Tests {
    using namespace Moment::Locality;

    TEST(Scenarios_Locality_CollinsGisin, CHSH) {
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

        system.RefreshCollinsGisin();
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

        const auto& s_id = *system.Symbols().where(os_id);
        const auto& s_a0 = *system.Symbols().where(os_a0);
        const auto& s_a1 = *system.Symbols().where(os_a1);
        const auto& s_b0 = *system.Symbols().where(os_b0);
        const auto& s_b1 = *system.Symbols().where(os_b1);
        const auto& s_a0b0 = *system.Symbols().where(os_a0b0);
        const auto& s_a0b1 = *system.Symbols().where(os_a0b1);
        const auto& s_a1b0 = *system.Symbols().where(os_a1b0);
        const auto& s_a1b1 = *system.Symbols().where(os_a1b1);


        ASSERT_EQ(cgi.Data().size(), 9);
        
        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{0, 0}), 0);
        EXPECT_EQ(cgi.Data()[0].symbol_id, s_id.Id());
        EXPECT_EQ(cgi.Data()[0].sequence, os_id);
        EXPECT_EQ(cgi.Data()[0].real_index, s_id.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{1, 0}), 1);
        EXPECT_EQ(cgi.Data()[1].symbol_id, s_a0.Id());
        EXPECT_EQ(cgi.Data()[1].sequence, os_a0);
        EXPECT_EQ(cgi.Data()[1].real_index, s_a0.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{2, 0}), 2);
        EXPECT_EQ(cgi.Data()[2].symbol_id, s_a1.Id());
        EXPECT_EQ(cgi.Data()[2].sequence, os_a1);
        EXPECT_EQ(cgi.Data()[2].real_index, s_a1.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{0, 1}), 3);
        EXPECT_EQ(cgi.Data()[3].symbol_id, s_b0.Id());
        EXPECT_EQ(cgi.Data()[3].sequence, os_b0);
        EXPECT_EQ(cgi.Data()[3].real_index, s_b0.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{1, 1}), 4);
        EXPECT_EQ(cgi.Data()[4].symbol_id, s_a0b0.Id());
        EXPECT_EQ(cgi.Data()[4].sequence, os_a0b0);
        EXPECT_EQ(cgi.Data()[4].real_index, s_a0b0.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{2, 1}), 5);
        EXPECT_EQ(cgi.Data()[5].symbol_id, s_a1b0.Id());
        EXPECT_EQ(cgi.Data()[5].sequence, os_a1b0);
        EXPECT_EQ(cgi.Data()[5].real_index, s_a1b0.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{0, 2}), 6);
        EXPECT_EQ(cgi.Data()[6].symbol_id, s_b1.Id());
        EXPECT_EQ(cgi.Data()[6].sequence, os_b1);
        EXPECT_EQ(cgi.Data()[6].real_index, s_b1.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{1, 2}), 7);
        EXPECT_EQ(cgi.Data()[7].symbol_id, s_a0b1.Id());
        EXPECT_EQ(cgi.Data()[7].sequence, os_a0b1);
        EXPECT_EQ(cgi.Data()[7].real_index, s_a0b1.basis_key().first);

        EXPECT_EQ(cgi.index_to_offset(CollinsGisinIndex{2, 2}), 8);
        EXPECT_EQ(cgi.Data()[8].symbol_id, s_a1b1.Id());
        EXPECT_EQ(cgi.Data()[8].sequence, os_a1b1);
        EXPECT_EQ(cgi.Data()[8].real_index, s_a1b1.basis_key().first);

    }

    TEST(Scenarios_Locality_CollinsGisin, PartialFilling_CHSH) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 2);
        ASSERT_EQ(bob.size(), 2);

        const auto& a0 = alice[0];
        const auto& a1 = alice[1];
        const auto& b0 = bob[0];
        const auto& b1 = bob[1];
        const OperatorSequence os_id{{}, context};
        const OperatorSequence os_a0{{a0}, context};
        const OperatorSequence os_a1{{a1}, context};
        const OperatorSequence os_b0{{b0}, context};
        const OperatorSequence os_b1{{b1}, context};
        const OperatorSequence os_a0b0{{a0, b0}, context};
        const OperatorSequence os_a0b1{{a0, b1}, context};
        const OperatorSequence os_a1b0{{a1, b0}, context};
        const OperatorSequence os_a1b1{{a1, b1}, context};

        system.RefreshCollinsGisin();
        const auto& cgi = system.CollinsGisin();
        ASSERT_FALSE(cgi.HasAllSymbols());
        ASSERT_EQ(cgi.Dimensions.size(), 2);
        ASSERT_EQ(cgi.Dimensions[0], 3);
        ASSERT_EQ(cgi.Dimensions[1], 3);
        ASSERT_EQ(cgi.Data().size(), 9);

        EXPECT_EQ(cgi.Data()[0].sequence, os_id);
        EXPECT_EQ(cgi.Data()[1].sequence, os_a0);
        EXPECT_EQ(cgi.Data()[2].sequence, os_a1);
        EXPECT_EQ(cgi.Data()[3].sequence, os_b0);
        EXPECT_EQ(cgi.Data()[4].sequence, os_a0b0);
        EXPECT_EQ(cgi.Data()[5].sequence, os_a1b0);
        EXPECT_EQ(cgi.Data()[6].sequence, os_b1);
        EXPECT_EQ(cgi.Data()[7].sequence, os_a0b1);
        EXPECT_EQ(cgi.Data()[8].sequence, os_a1b1);

        system.generate_dictionary(2);
        system.RefreshCollinsGisin();
        ASSERT_TRUE(cgi.HasAllSymbols());
        const auto& s_id = *system.Symbols().where(os_id);
        const auto& s_a0 = *system.Symbols().where(os_a0);
        const auto& s_a1 = *system.Symbols().where(os_a1);
        const auto& s_b0 = *system.Symbols().where(os_b0);
        const auto& s_b1 = *system.Symbols().where(os_b1);
        const auto& s_a0b0 = *system.Symbols().where(os_a0b0);
        const auto& s_a0b1 = *system.Symbols().where(os_a0b1);
        const auto& s_a1b0 = *system.Symbols().where(os_a1b0);
        const auto& s_a1b1 = *system.Symbols().where(os_a1b1);


        ASSERT_EQ(cgi.Data().size(), 9);
        EXPECT_EQ(cgi.Data()[0].symbol_id, s_id.Id());
        EXPECT_EQ(cgi.Data()[0].real_index, s_id.basis_key().first);

        EXPECT_EQ(cgi.Data()[1].symbol_id, s_a0.Id());
        EXPECT_EQ(cgi.Data()[1].real_index, s_a0.basis_key().first);

        EXPECT_EQ(cgi.Data()[2].symbol_id, s_a1.Id());
        EXPECT_EQ(cgi.Data()[2].real_index, s_a1.basis_key().first);

        EXPECT_EQ(cgi.Data()[3].symbol_id, s_b0.Id());
        EXPECT_EQ(cgi.Data()[3].real_index, s_b0.basis_key().first);

        EXPECT_EQ(cgi.Data()[4].symbol_id, s_a0b0.Id());
        EXPECT_EQ(cgi.Data()[4].real_index, s_a0b0.basis_key().first);

        EXPECT_EQ(cgi.Data()[5].symbol_id, s_a1b0.Id());
        EXPECT_EQ(cgi.Data()[5].real_index, s_a1b0.basis_key().first);

        EXPECT_EQ(cgi.Data()[6].symbol_id, s_b1.Id());
        EXPECT_EQ(cgi.Data()[6].real_index, s_b1.basis_key().first);

        EXPECT_EQ(cgi.Data()[7].symbol_id, s_a0b1.Id());
        EXPECT_EQ(cgi.Data()[7].real_index, s_a0b1.basis_key().first);

        EXPECT_EQ(cgi.Data()[8].symbol_id, s_a1b1.Id());
        EXPECT_EQ(cgi.Data()[8].real_index, s_a1b1.basis_key().first);


    }

    TEST(Scenarios_Locality_CollinsGisin, BadIndices) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& mm = system.create_moment_matrix(1);
        system.RefreshCollinsGisin();
        const auto& cgi = system.CollinsGisin();

        std::vector<size_t> biA = {1};
        EXPECT_THROW(cgi.validate_index(biA), Moment::errors::bad_tensor_index);

        std::vector<size_t> biB = {1, 1, 1};
        EXPECT_THROW(cgi.validate_index(biA), Moment::errors::bad_tensor_index);

        std::vector<size_t> biC = {3, 1};
        EXPECT_THROW(cgi.validate_index(biA), Moment::errors::bad_tensor_index);

        std::vector<size_t> biD = {1, 3};
        EXPECT_THROW(cgi.validate_index(biA), Moment::errors::bad_tensor_index);
    }

    TEST(Scenarios_Locality_CollinsGisin, Range_CHSH) {
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

        system.RefreshCollinsGisin();
        const auto& cgi = system.CollinsGisin();
        ASSERT_TRUE(cgi.HasAllSymbols());

        const OperatorSequence os_id{{}, context};
        const OperatorSequence os_a0{{a0}, context};
        const OperatorSequence os_a1{{a1}, context};
        const OperatorSequence os_b0{{b0}, context};
        const OperatorSequence os_b1{{b1}, context};
        const OperatorSequence os_a0b0{{a0, b0}, context};
        const OperatorSequence os_a0b1{{a0, b1}, context};
        const OperatorSequence os_a1b0{{a1, b0}, context};
        const OperatorSequence os_a1b1{{a1, b1}, context};

        const auto& s_id = *system.Symbols().where(os_id);
        const auto& s_a0 = *system.Symbols().where(os_a0);
        const auto& s_a1 = *system.Symbols().where(os_a1);
        const auto& s_b0 = *system.Symbols().where(os_b0);
        const auto& s_b1 = *system.Symbols().where(os_b1);
        const auto& s_a0b0 = *system.Symbols().where(os_a0b0);
        const auto& s_a0b1 = *system.Symbols().where(os_a0b1);
        const auto& s_a1b0 = *system.Symbols().where(os_a1b0);
        const auto& s_a1b1 = *system.Symbols().where(os_a1b1);

        // Test 'A0' measurement, composed of one operator a0.
        auto a0_range = cgi.measurement_to_range(std::vector<size_t>{0}); // A0 mmt.
        auto a0_iter = a0_range.begin();
        ASSERT_TRUE(a0_iter.operator bool());
        ASSERT_FALSE(!a0_iter);
        ASSERT_NE(a0_iter, a0_range.end());
        EXPECT_EQ(a0_iter.block_index(), 0);
        EXPECT_EQ(a0_iter.sequence(), os_a0);
        EXPECT_EQ(a0_iter.symbol_id(), s_a0.Id());
        EXPECT_EQ(a0_iter.real_basis(), s_a0.basis_key().first);
        ++a0_iter;
        EXPECT_EQ(a0_iter, a0_range.end());

        // Test 'B1' measurement, composed of one operator b0.
        auto b1_range = cgi.measurement_to_range(std::vector<size_t>{3}); // B1 mmt.
        auto b1_iter = b1_range.begin();
        ASSERT_TRUE(b1_iter.operator bool());
        ASSERT_FALSE(!b1_iter);
        ASSERT_NE(b1_iter, b1_range.end());
        EXPECT_EQ(b1_iter.block_index(), 0);
        EXPECT_EQ(b1_iter.sequence(), os_b1);
        EXPECT_EQ(b1_iter.symbol_id(), s_b1.Id());
        EXPECT_EQ(b1_iter.real_basis(), s_b1.basis_key().first);
        ++b1_iter;
        EXPECT_EQ(b1_iter, b1_range.end());

        // Test 'A0B1' measurement, composed of one operator a0b1
        auto a0b1_range = cgi.measurement_to_range(std::vector<size_t>{0, 3}); // A0, B1 mmt
        auto a0b1_iter = a0b1_range.begin();
        ASSERT_TRUE(a0b1_iter.operator bool());
        ASSERT_FALSE(!a0b1_iter);
        ASSERT_NE(a0b1_iter, a0b1_range.end());
        EXPECT_EQ(a0b1_iter.block_index(), 0);
        EXPECT_EQ(a0b1_iter.sequence(), os_a0b1);
        EXPECT_EQ(a0b1_iter.symbol_id(), s_a0b1.Id());
        EXPECT_EQ(a0b1_iter.real_basis(), s_a0b1.basis_key().first);
        ++a0b1_iter;
        EXPECT_EQ(a0b1_iter, a0b1_range.end());
    }


    TEST(Scenarios_Locality_CollinsGisin, Range_ThreeOutputs) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 3))};
        const auto& context = system.localityContext;
        system.generate_dictionary(2);

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 4);
        ASSERT_EQ(bob.size(), 4);

        const auto& A_a0 = alice[0];
        const auto& A_a1 = alice[1];
        const auto& A_b0 = alice[2];
        const auto& A_b1 = alice[3];
        const auto& B_a0 = bob[0];
        const auto& B_a1 = bob[1];
        const auto& B_b0 = bob[2];
        const auto& B_b1 = bob[3];

        system.RefreshCollinsGisin();
        const auto& cgi = system.CollinsGisin();
        ASSERT_TRUE(cgi.HasAllSymbols());

          const OperatorSequence os_A_a0{{A_a0}, context};
          const OperatorSequence os_A_a1{{A_a1}, context};
          const OperatorSequence os_B_b0{{B_b0}, context};
          const OperatorSequence os_B_b1{{B_b1}, context};

          const OperatorSequence os_Aa0_Bb0{{A_a0, B_b0}, context};
          const OperatorSequence os_Aa0_Bb1{{A_a0, B_b1}, context};
          const OperatorSequence os_Aa1_Bb0{{A_a1, B_b0}, context};
          const OperatorSequence os_Aa1_Bb1{{A_a1, B_b1}, context};


         const auto& s_A_a0 = *system.Symbols().where(os_A_a0);
         const auto& s_A_a1 = *system.Symbols().where(os_A_a1);
         const auto& s_B_b0 = *system.Symbols().where(os_B_b0);
         const auto& s_B_b1 = *system.Symbols().where(os_B_b1);

         const auto& s_Aa0_Bb0 = *system.Symbols().where(os_Aa0_Bb0);
         const auto& s_Aa0_Bb1 = *system.Symbols().where(os_Aa0_Bb1);
         const auto& s_Aa1_Bb0 = *system.Symbols().where(os_Aa1_Bb0);
         const auto& s_Aa1_Bb1 = *system.Symbols().where(os_Aa1_Bb1);

        // Test 'A0' measurement, composed of two operators A_a0, A_a1
        auto A0_range = cgi.measurement_to_range(std::vector<size_t>{0}); // A0 mmt.
        auto A0_iter = A0_range.begin();
        ASSERT_TRUE(A0_iter.operator bool());
        ASSERT_FALSE(!A0_iter);
        ASSERT_NE(A0_iter, A0_range.end());
        EXPECT_EQ(A0_iter.block_index(), 0);
        EXPECT_EQ(A0_iter.sequence(), os_A_a0);
        EXPECT_EQ(A0_iter.symbol_id(), s_A_a0.Id());
        EXPECT_EQ(A0_iter.real_basis(), s_A_a0.basis_key().first);
        ++A0_iter;

        ASSERT_NE(A0_iter, A0_range.end());
        EXPECT_EQ(A0_iter.block_index(), 1);
        EXPECT_EQ(A0_iter.sequence(), os_A_a1);
        EXPECT_EQ(A0_iter.symbol_id(), s_A_a1.Id());
        EXPECT_EQ(A0_iter.real_basis(), s_A_a1.basis_key().first);
        ++A0_iter;

        EXPECT_EQ(A0_iter, A0_range.end());

        // Test 'B1' measurement, composed of two operators B_b0, B_b1
        auto B1_range = cgi.measurement_to_range(std::vector<size_t>{3}); // B1 mmt.
        auto B1_iter = B1_range.begin();
        ASSERT_TRUE(B1_iter.operator bool());
        ASSERT_FALSE(!B1_iter);
        ASSERT_NE(B1_iter, B1_range.end());
        EXPECT_EQ(B1_iter.block_index(), 0);
        EXPECT_EQ(B1_iter.sequence(), os_B_b0);
        EXPECT_EQ(B1_iter.symbol_id(), s_B_b0.Id());
        EXPECT_EQ(B1_iter.real_basis(), s_B_b0.basis_key().first);
        ++B1_iter;

        ASSERT_NE(B1_iter, B1_range.end());
        EXPECT_EQ(B1_iter.block_index(), 1);
        EXPECT_EQ(B1_iter.sequence(), os_B_b1);
        EXPECT_EQ(B1_iter.symbol_id(), s_B_b1.Id());
        EXPECT_EQ(B1_iter.real_basis(), s_B_b1.basis_key().first);
        ++B1_iter;

        EXPECT_EQ(B1_iter, B1_range.end());


        // Test 'A0B1' measurement explicit, composed of four operators A_a0 B_b0, A_a0 B_b1, A_a1 B_b0, A_a1 B_b1
        auto A0B1_range = cgi.measurement_to_range(std::vector<size_t>{0, 3}); // A0 B1 joint measurement
        auto A0B1_iter = A0B1_range.begin();
        ASSERT_TRUE(A0B1_iter.operator bool());
        ASSERT_FALSE(!A0B1_iter);
        ASSERT_NE(A0B1_iter, A0B1_range.end());
        EXPECT_EQ(A0B1_iter.block_index(), 0);
        EXPECT_EQ(A0B1_iter.sequence(), os_Aa0_Bb0);
        EXPECT_EQ(A0B1_iter.symbol_id(), s_Aa0_Bb0.Id());
        EXPECT_EQ(A0B1_iter.real_basis(), s_Aa0_Bb0.basis_key().first);
        ++A0B1_iter;

        ASSERT_NE(A0B1_iter, A0B1_range.end());
        EXPECT_EQ(A0B1_iter.block_index(), 1);
        EXPECT_EQ(A0B1_iter.sequence(), os_Aa1_Bb0);
        EXPECT_EQ(A0B1_iter.symbol_id(), s_Aa1_Bb0.Id());
        EXPECT_EQ(A0B1_iter.real_basis(), s_Aa1_Bb0.basis_key().first);
        ++A0B1_iter;

        ASSERT_NE(A0B1_iter, A0B1_range.end());
        EXPECT_EQ(A0B1_iter.block_index(), 2);
        EXPECT_EQ(A0B1_iter.sequence(), os_Aa0_Bb1);
        EXPECT_EQ(A0B1_iter.symbol_id(), s_Aa0_Bb1.Id());
        EXPECT_EQ(A0B1_iter.real_basis(), s_Aa0_Bb1.basis_key().first);
        ++A0B1_iter;

        ASSERT_NE(A0B1_iter, A0B1_range.end());
        EXPECT_EQ(A0B1_iter.block_index(), 3);
        EXPECT_EQ(A0B1_iter.sequence(), os_Aa1_Bb1);
        EXPECT_EQ(A0B1_iter.symbol_id(), s_Aa1_Bb1.Id());
        EXPECT_EQ(A0B1_iter.real_basis(), s_Aa1_Bb1.basis_key().first);
        ++A0B1_iter;

        EXPECT_EQ(A0B1_iter, A0B1_range.end());


        // Test 'A0B1' measurement with B1= outcome 1, composed of four operators  A_a0 B_b1, A_a1 B_b1
        auto A0B1_fix_range = cgi.measurement_to_range(std::vector<size_t>{0, 3}, std::vector<oper_name_t>{-1, 1});
        auto A0B1_fix_iter = A0B1_fix_range.begin();
        ASSERT_TRUE(A0B1_fix_iter.operator bool());
        ASSERT_FALSE(!A0B1_fix_iter);
        ASSERT_NE(A0B1_fix_iter, A0B1_fix_range.end());
        EXPECT_EQ(A0B1_fix_iter.block_index(), 0);
        EXPECT_EQ(A0B1_fix_iter.sequence(), os_Aa0_Bb1);
        EXPECT_EQ(A0B1_fix_iter.symbol_id(), s_Aa0_Bb1.Id());
        EXPECT_EQ(A0B1_fix_iter.real_basis(), s_Aa0_Bb1.basis_key().first);
        ++A0B1_fix_iter;

        ASSERT_NE(A0B1_fix_iter, A0B1_fix_range.end());
        EXPECT_EQ(A0B1_fix_iter.block_index(), 1);
        EXPECT_EQ(A0B1_fix_iter.sequence(), os_Aa1_Bb1);
        EXPECT_EQ(A0B1_fix_iter.symbol_id(), s_Aa1_Bb1.Id());
        EXPECT_EQ(A0B1_fix_iter.real_basis(), s_Aa1_Bb1.basis_key().first);
        ++A0B1_fix_iter;

        EXPECT_EQ(B1_iter, A0B1_fix_range.end());


    }


}