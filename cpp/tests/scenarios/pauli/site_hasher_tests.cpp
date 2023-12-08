/**
 * site_hasher_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/site_hasher.h"

#include <array>
#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_SiteHasher, Hash_SmallEmpty) {
        PauliContext empty{0};
        SiteHasher<1> hasher{empty};
        EXPECT_EQ(sizeof(SiteHasher<1>::Datum), 8);
        EXPECT_EQ(SiteHasher<1>::qubits_per_slide, 32);

        EXPECT_EQ(hasher({}), 0);
    }

    TEST(Scenarios_Pauli_SiteHasher, Hash_Small) {
        PauliContext context{5};
        SiteHasher<1> hasher{context};
        EXPECT_EQ(hasher.qubits, 5);
        EXPECT_EQ(hasher.column_height, 5);
        EXPECT_EQ(hasher.row_width, 1);

        EXPECT_EQ(hasher(context.identity()),0x0000000000000000);
        EXPECT_EQ(hasher(context.sigmaX(0)), 0x0000000000000001);
        EXPECT_EQ(hasher(context.sigmaY(0)), 0x0000000000000002);
        EXPECT_EQ(hasher(context.sigmaZ(0)), 0x0000000000000003);

        EXPECT_EQ(hasher(context.sigmaX(1)), 0x0000000000000004);
        EXPECT_EQ(hasher(context.sigmaY(1)), 0x0000000000000008);
        EXPECT_EQ(hasher(context.sigmaZ(1)), 0x000000000000000c);

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(1))), 0x0000000000000005);
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaY(1))), 0x0000000000000009);
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaZ(1))), 0x000000000000000d);
    }

    TEST(Scenarios_Pauli_SiteHasher, Hash_Medium) {
        PauliContext context{40};
        SiteHasher<2> hasher{context};
        ASSERT_EQ(hasher.qubits_per_slide, 32);

        EXPECT_EQ(hasher(context.identity()),
                  (std::array<uint64_t, 2>{0x0000000000000000, 0}));

        EXPECT_EQ(hasher(context.sigmaX(0)),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0}));
        EXPECT_EQ(hasher(context.sigmaY(0)),
                  (std::array<uint64_t, 2>{0x0000000000000002, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(0)),
                  (std::array<uint64_t, 2>{0x0000000000000003, 0}));


        EXPECT_EQ(hasher(context.sigmaX(16)),
                  (std::array<uint64_t, 2>{0x0000000100000000, 0}));
        EXPECT_EQ(hasher(context.sigmaY(16)),
                  (std::array<uint64_t, 2>{0x0000000200000000, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(16)),
                  (std::array<uint64_t, 2>{ 0x0000000300000000, 0}));


        EXPECT_EQ(hasher(context.sigmaX(32)),
                  (std::array<uint64_t, 2>{0, 0x0000000000000001}));
        EXPECT_EQ(hasher(context.sigmaY(32)),
                  (std::array<uint64_t, 2>{0, 0x0000000000000002}));
        EXPECT_EQ(hasher(context.sigmaZ(32)),
                  (std::array<uint64_t, 2>{0, 0x0000000000000003}));


        EXPECT_EQ(hasher(context.sigmaX(33)),
                  (std::array<uint64_t, 2>{0, 0x0000000000000004}));
        EXPECT_EQ(hasher(context.sigmaY(33)),
                  (std::array<uint64_t, 2>{0, 0x0000000000000008}));
        EXPECT_EQ(hasher(context.sigmaZ(33)),
                  (std::array<uint64_t, 2>{0, 0x000000000000000c}));

        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaX(33))),
                  (std::array<uint64_t, 2>{0, 0x0000000000000005}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaY(33))),
                  (std::array<uint64_t, 2>{0, 0x0000000000000009}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaZ(33))),
                  (std::array<uint64_t, 2>{0, 0x000000000000000d}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(33))),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0x0000000000000004}));
        EXPECT_EQ(hasher((context.sigmaX(1) * context.sigmaY(33))),
                  (std::array<uint64_t, 2>{0x0000000000000004, 0x0000000000000008}));
        EXPECT_EQ(hasher((context.sigmaX(2) * context.sigmaZ(33))),
                  (std::array<uint64_t, 2>{0x0000000000000010, 0x000000000000000c}));
    }


    TEST(Scenarios_Pauli_SiteHasher, Hash_Larger) {
        PauliContext context{70};
        SiteHasher<3> hasher{context};
        ASSERT_EQ(hasher.qubits_per_slide, 32);
        
        EXPECT_EQ(hasher(context.identity()),
                  (std::array<uint64_t, 3>{0x0000000000000000, 0, 0}));

        EXPECT_EQ(hasher(context.sigmaX(0)),
                  (std::array<uint64_t, 3>{0x0000000000000001, 0, 0}));
        EXPECT_EQ(hasher(context.sigmaY(0)),
                  (std::array<uint64_t, 3>{0x0000000000000002, 0, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(0)),
                  (std::array<uint64_t, 3>{0x0000000000000003, 0, 0}));


        EXPECT_EQ(hasher(context.sigmaX(16)),
                  (std::array<uint64_t, 3>{0x0000000100000000, 0, 0}));
        EXPECT_EQ(hasher(context.sigmaY(16)),
                  (std::array<uint64_t, 3>{0x0000000200000000, 0, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(16)),
                  (std::array<uint64_t, 3>{ 0x0000000300000000, 0, 0}));


        EXPECT_EQ(hasher(context.sigmaX(32)),
                  (std::array<uint64_t, 3>{0, 0x0000000000000001, 0}));
        EXPECT_EQ(hasher(context.sigmaY(32)),
                  (std::array<uint64_t, 3>{0, 0x0000000000000002, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(32)),
                  (std::array<uint64_t, 3>{0, 0x0000000000000003, 0}));

        EXPECT_EQ(hasher(context.sigmaX(64)),
                  (std::array<uint64_t, 3>{0, 0, 0x0000000000000001}));
        EXPECT_EQ(hasher(context.sigmaY(64)),
                  (std::array<uint64_t, 3>{0, 0, 0x0000000000000002}));
        EXPECT_EQ(hasher(context.sigmaZ(64)),
                  (std::array<uint64_t, 3>{0, 0, 0x0000000000000003}));

        EXPECT_EQ(hasher(context.sigmaX(33)),
                  (std::array<uint64_t, 3>{0, 0x0000000000000004, 0}));
        EXPECT_EQ(hasher(context.sigmaY(33)),
                  (std::array<uint64_t, 3>{0, 0x0000000000000008, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(33)),
                  (std::array<uint64_t, 3>{0, 0x000000000000000c, 0}));

        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaX(33))),
                  (std::array<uint64_t, 3>{0, 0x0000000000000005, 0}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaY(33))),
                  (std::array<uint64_t, 3>{0, 0x0000000000000009, 0}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaZ(33))),
                  (std::array<uint64_t, 3>{0, 0x000000000000000d, 0}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(33))),
                  (std::array<uint64_t, 3>{0x0000000000000001, 0x0000000000000004, 0}));
        EXPECT_EQ(hasher((context.sigmaX(1) * context.sigmaY(33))),
                  (std::array<uint64_t, 3>{0x0000000000000004, 0x0000000000000008, 0}));
        EXPECT_EQ(hasher((context.sigmaX(2) * context.sigmaZ(33))),
                  (std::array<uint64_t, 3>{0x0000000000000010, 0x000000000000000c, 0}));
    }

    TEST(Scenarios_Pauli_SiteHasher, Unhash_SmallChain) {
        PauliContext context{5};
        SiteHasher<1> hasher{context};

        // Single qubits
        for (size_t q = 0; q < 5; ++q) {
            // Sigma X
            auto sequenceX = context.sigmaX(q);
            auto hashX = hasher.hash(sequenceX);
            OperatorSequence reconstructX{hasher.unhash(hashX), context};
            EXPECT_EQ(reconstructX, sequenceX) << "q = " << q;

            // Sigma Y
            auto sequenceY = context.sigmaY(q);
            auto hashY = hasher.hash(sequenceY);
            OperatorSequence reconstructY{hasher.unhash(hashY), context};
            EXPECT_EQ(reconstructY, sequenceY) << "q = " << q;

            // Sigma Z
            auto sequenceZ = context.sigmaZ(q);
            auto hashZ = hasher.hash(sequenceZ);
            OperatorSequence reconstructZ{hasher.unhash(hashZ), context};
            EXPECT_EQ(reconstructZ, sequenceZ) << "q = " << q;
        }

        // Pairs of qubits
        for (size_t q = 0; q < 5; ++q) {
            for (size_t r = 0; r < 5; ++r) {
                if (q == r) {
                    continue; // Skip XZ = +iY
                }
                // Sigma Xq * Zr
                auto sequenceXZ = context.sigmaX(q) * context.sigmaZ(r);
                auto hashXZ = hasher.hash(sequenceXZ);
                OperatorSequence reconstructXZ{hasher.unhash(hashXZ), context};
                EXPECT_EQ(reconstructXZ, sequenceXZ) << "q = " << q << ", r = " << r;
            }
        }

        // Five qubits
        OperatorSequence seqXXXXX = context.sigmaX(0) * context.sigmaX(1) * context.sigmaX(2)
                                    * context.sigmaX(3) * context.sigmaX(4);
        auto hashXXXXX = hasher.hash(seqXXXXX);
        OperatorSequence reconstructXXXXX{hasher.unhash(hashXXXXX), context};
        EXPECT_EQ(reconstructXXXXX, seqXXXXX);
    }

    TEST(Scenarios_Pauli_SiteHasher, Unhash_MediumChain) {
        PauliContext context{40};
        SiteHasher<2> hasher{context};

        // Single qubits
        for (size_t q = 0; q < 40; ++q) {
            // Sigma X
            auto sequenceX = context.sigmaX(q);
            auto hashX = hasher.hash(sequenceX);
            OperatorSequence reconstructX{hasher.unhash(hashX), context};
            EXPECT_EQ(reconstructX, sequenceX) << "q = " << q;

            // Sigma Y
            auto sequenceY = context.sigmaY(q);
            auto hashY = hasher.hash(sequenceY);
            OperatorSequence reconstructY{hasher.unhash(hashY), context};
            EXPECT_EQ(reconstructY, sequenceY) << "q = " << q;

            // Sigma Z
            auto sequenceZ = context.sigmaZ(q);
            auto hashZ = hasher.hash(sequenceZ);
            OperatorSequence reconstructZ{hasher.unhash(hashZ), context};
            EXPECT_EQ(reconstructZ, sequenceZ) << "q = " << q;
        }

        // Pairs of qubits
        for (size_t q = 1; q < 40; ++q) {
            // Sigma X1 * Zq
            auto sequenceXZ = context.sigmaX(0) * context.sigmaZ(q);
            auto hashXZ = hasher.hash(sequenceXZ);
            OperatorSequence reconstructXZ{hasher.unhash(hashXZ), context};
            EXPECT_EQ(reconstructXZ, sequenceXZ) << "q = " << q;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_SmallAligned) {
        PauliContext context{32};
        SiteHasher<1> hasher{context};
        EXPECT_EQ(hasher.final_slide_mask, 0xffffffffffffffff);

        // Small shift
        for (size_t shift_index = 0; shift_index < 32; ++shift_index) {
            // One qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;

            // Two qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%32))))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_SmallUnaligned) {
        PauliContext context{25};
        SiteHasher<1> hasher{context};
        EXPECT_EQ(hasher.final_slide_mask, 0x0003ffffffffffff);

        // Small shift
        for (size_t shift_index = 0; shift_index < 25; ++shift_index) {
            // One qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;

            // Two qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%25))))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_MediumAligned) {
        PauliContext context{64};
        ASSERT_EQ(context.qubit_size, 64);
        SiteHasher<2> hasher{context};
        ASSERT_EQ(hasher.impl_label, 2);
        EXPECT_EQ(hasher.qubits, 64);
        EXPECT_EQ(hasher.column_height, 64);
        EXPECT_EQ(hasher.row_width, 1);
        EXPECT_EQ(hasher.qubits_per_slide, 32);
        EXPECT_EQ(hasher.final_slide_mask, 0xffffffffffffffff);
        EXPECT_EQ(hasher.qubits_on_final_slide, 32);


        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%64))))
                                << " shift_index = " << shift_index;
        }

        // Trickier two qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(31))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+31)%64))))
                                << " shift_index = " << shift_index;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_MediumUnaligned) {
        PauliContext context{40};
        SiteHasher<2> hasher{context}; // So, 8 qubits [16 bits] on second page
        ASSERT_EQ(hasher.final_slide_mask, 0x000000000000ffff);
        ASSERT_EQ(hasher.qubits_on_final_slide, 8);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%40))))
                      << " shift_index = " << shift_index;
        }

        // Trickier two qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(31))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+31)%40))))
                      << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_LargerAligned) {
        PauliContext context{96};
        SiteHasher<3> hasher{context}; // So, 16 qubits [32 bits] on final page
        ASSERT_EQ(hasher.qubits_on_final_slide, 32);
        ASSERT_EQ(hasher.final_slide_mask, 0xffffffffffffffff);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 96; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 96; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaZ(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaZ((shift_index + 3) % 96))))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_LargerUnaligned) {
        PauliContext context{80};
        SiteHasher<3> hasher{context}; // So, 16 qubits [32 bits] on final page
        ASSERT_EQ(hasher.qubits_on_final_slide, 16);
        ASSERT_EQ(hasher.final_slide_mask, 0x00000000ffffffff);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 80; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0)), shift_index),
                      hasher(context.sigmaX(shift_index))) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 80; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3))), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index + 3) % 80))))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ColShift_Small) {
        PauliContext context{4, 2, true, true}; // 4x2 wrapping grid
        SiteHasher<1> hasher{context};
        ASSERT_EQ(hasher.qubits, 8);
        ASSERT_EQ(hasher.column_height, 4);
        ASSERT_EQ(hasher.row_width, 2);

        for (size_t row_id = 0; row_id < 4; ++row_id) {
            EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 0)), 0),
                      hasher(context.sigmaX(row_id, 0))) << "row = " << row_id;
            EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 0)), 1),
                      hasher(context.sigmaX(row_id, 1))) << "row = " << row_id;
            EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 1)), 1),
                      hasher(context.sigmaX(row_id, 0))) << "row = " << row_id;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ColShift_Medium) {
        PauliContext context{8, 5, true, true}; // 8x5 wrapping grid
        SiteHasher<2> hasher{context};
        ASSERT_EQ(hasher.qubits, 40);
        ASSERT_EQ(hasher.column_height, 8);
        ASSERT_EQ(hasher.row_width, 5);

        for (size_t row_id = 0; row_id < 8; ++row_id) {
            for (size_t shift = 0; shift < 5; ++shift) {
                EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 0)), shift),
                          hasher(context.sigmaX(row_id, shift % 5))) << "row = " << row_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ColShift_Larger) {
        PauliContext context{4, 5, true, true}; // 4x5 wrapping grid
        SiteHasher<3> hasher{context};
        ASSERT_EQ(hasher.column_height, 4);
        ASSERT_EQ(hasher.row_width, 5);

        for (size_t row_id = 0; row_id < 4; ++row_id) {
            for (size_t shift = 0; shift < 5; ++shift) {
                EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 0)), shift),
                          hasher(context.sigmaX(row_id, shift % 5))) << "row = " << row_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ExtractColumn_MediumAligned) {
        PauliContext context{8, 8, true, true}; // 8x8 grid
        SiteHasher<2> hasher{context};
        ASSERT_EQ(hasher.column_height, 8);
        ASSERT_EQ(hasher.row_width, 8);

        EXPECT_EQ(hasher.boundary_info.wrap_column, 4); // col 4 begins at 0 of RHS
        EXPECT_EQ(hasher.boundary_info.lhs_anti_offset, 64);
        EXPECT_EQ(hasher.boundary_info.lhs_mask, 0x0000000000000000);
        EXPECT_EQ(hasher.boundary_info.rhs_offset, 0);
        EXPECT_EQ(hasher.boundary_info.rhs_mask, 0x0000000000000ffff); // first 16 bits set

        for (size_t col_id = 0; col_id < 8; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                                << "col = " << col_id;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ExtractColumn_MediumUnaligned) {
        PauliContext context{5, 10, true, true}; // 10x5 grid
        SiteHasher<2> hasher{context};
        ASSERT_EQ(hasher.column_height, 5);
        ASSERT_EQ(hasher.row_width, 10);

        EXPECT_EQ(hasher.boundary_info.wrap_column, 6); // col 6 begins at 60 (inc.) of LHS
        EXPECT_EQ(hasher.boundary_info.lhs_anti_offset, 60); // begins at 60 (inc.) of LHS
        EXPECT_EQ(hasher.boundary_info.lhs_mask,   0xf000000000000000); // last first 4 bits set
        EXPECT_EQ(hasher.boundary_info.rhs_offset, 4); // Make space for those 2 qubits on LHS.
        EXPECT_EQ(hasher.boundary_info.rhs_mask, 0x0000000000000003f); // first 6 bits set

        for (size_t col_id = 0; col_id < 10; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                << "col = " << col_id;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ExtractColumn_LargerAligned) {
        PauliContext context{8, 12, true, true}; // 8x12 grid
        SiteHasher<3> hasher{context};
        ASSERT_EQ(hasher.column_height, 8);
        ASSERT_EQ(hasher.row_width, 12);

        for (size_t col_id = 0; col_id < 12; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                << "col = " << col_id;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, ExtractColumn_LargerUnaligned) {
        PauliContext context{5, 14, true, true}; // 5x14 grid
        SiteHasher<3> hasher{context};
        ASSERT_EQ(hasher.column_height, 5);
        ASSERT_EQ(hasher.row_width, 14);

        for (size_t col_id = 0; col_id < 14; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                << "col = " << col_id;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, RowCyclicShift_Small) {
        PauliContext context{4, 2, true, true}; // 4x2 wrapping grid
        SiteHasher<1> hasher{context};

        for (size_t row_id = 0; row_id < 4; ++row_id) {
            EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, 0)), 0),
                      hasher(context.sigmaX(row_id, 0))) << "row = " << row_id;
            EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, 0)), 1),
                      hasher(context.sigmaX((row_id+1)%4, 0))) << "row = " << row_id;
            EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, 1)), 1),
                      hasher(context.sigmaX((row_id+1)%4, 1))) << "row = " << row_id;
        }
    }


    TEST(Scenarios_Pauli_SiteHasher, RowCyclicShift_MediumUnaligned) {
        const size_t column_height = 12;
        const size_t column_count = 4;
        PauliContext context{column_height, column_count, true, true}; // 12x4 wrapping grid
        SiteHasher<2> hasher{context};
        EXPECT_EQ(hasher.column_height, 12);
        EXPECT_EQ(hasher.row_width, 4);


        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                          << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                          << "row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, RowCyclicShift_MediumAligned) {
        const size_t column_height = 8;
        const size_t column_count = 8;
        PauliContext context{8, 8, true, true}; // 8x8 wrapping grid
        SiteHasher<2> hasher{context};
        EXPECT_EQ(hasher.column_height, 8);
        EXPECT_EQ(hasher.row_width, 8);

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }


    TEST(Scenarios_Pauli_SiteHasher, RowCyclicShift_LargerAligned) {
        const size_t column_height = 8;
        const size_t column_count = 10;
        PauliContext context{column_height, column_count, true, true}; // 7x10 wrapping grid
        SiteHasher<3>  hasher{context};
        EXPECT_EQ(hasher.column_height, 8);
        EXPECT_EQ(hasher.row_width, 10);


        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, RowCyclicShift_LargerUnaligned) {
        const size_t column_height = 7;
        const size_t column_count = 10;
        PauliContext context{column_height, column_count, true, true}; // 7x10 wrapping grid
        SiteHasher<3> hasher{context};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_cyclic_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, LatticeShift_Small) {
        const size_t column_height = 4;
        const size_t column_count = 4;
        PauliContext context{column_height, column_count, true, true}; // 4x4 wrapping grid
        SiteHasher<1> hasher{context};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {

                // Single Pauli
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "Single, row = " << row_id << ", col = " << col_id;

                // X1 <-> Z2 Horizontal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaZ(0, 1)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaZ(row_id, (col_id+1)% column_count)))
                                    << "Horizontal, row = " << row_id << ", col = " << col_id;

                // X1 <-> Y5 Vertical
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaY(1, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaY((row_id +1)% column_height, col_id)))
                                    << "Vertical, row = " << row_id << ", col = " << col_id;

                // Y1 <-> X6 Diagonal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaY(0, 0) * context.sigmaX(1, 1)), row_id, col_id),
                          hasher(context.sigmaY(row_id, col_id)
                                 * context.sigmaX((row_id +1)% column_height, (col_id+1)% column_count)))
                                    << "Diagonal, row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, LatticeShift_Medium) {
        const size_t column_height = 6;
        const size_t column_count = 6;
        PauliContext context{column_height, column_count, true, true};  // 6x6 wrapping grid
        SiteHasher<2> hasher{context};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {

                // Single Pauli
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "Single, row = " << row_id << ", col = " << col_id;

                // X1 <-> Z2 Horizontal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaZ(0, 1)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaZ(row_id, (col_id+1)% column_count)))
                                    << "Horizontal, row = " << row_id << ", col = " << col_id;

                // X1 <-> Y5 Vertical
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaY(1, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaY((row_id +1)% column_height, col_id)))
                                    << "Vertical, row = " << row_id << ", col = " << col_id;

                // Y1 <-> X6 Diagonal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaY(0, 0) * context.sigmaX(1, 1)), row_id, col_id),
                          hasher(context.sigmaY(row_id, col_id)
                                 * context.sigmaX((row_id +1)% column_height, (col_id+1)% column_count)))
                                    << "Diagonal, row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, LatticeShift_Larger) {
        const size_t column_height = 9;
        const size_t column_count = 9;
        PauliContext context{column_height, column_count, true, true}; // 9x9 wrapping grid
        SiteHasher<3> hasher{context};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {

                // Single Pauli
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "Single, row = " << row_id << ", col = " << col_id;

                // X1 <-> Z5 Horizontal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaZ(0, 1)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaZ(row_id, (col_id+1)% column_count)))
                                    << "Horizontal, row = " << row_id << ", col = " << col_id;

                // X1 <-> Y2 Vertical
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaX(0, 0) * context.sigmaY(1, 0)), row_id, col_id),
                          hasher(context.sigmaX(row_id, col_id) * context.sigmaY((row_id +1)% column_height, col_id)))
                                    << "Vertical, row = " << row_id << ", col = " << col_id;

                // Y1 <-> X6 Diagonal
                EXPECT_EQ(hasher.lattice_shift(hasher(context.sigmaY(0, 0) * context.sigmaX(1, 1)), row_id, col_id),
                          hasher(context.sigmaY(row_id, col_id)
                                 * context.sigmaX((row_id +1)% column_height, (col_id+1)% column_count)))
                                    << "Diagonal, row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalHash_ChainSmall) {
        PauliContext context{5, true, true}; // 5-qubit chain
        SiteHasher<1> hasher{context};

        // Single qubits
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000001}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000004}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(2)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000010}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(3)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000040}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(4)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000100}));

        // Neighbouring pairs
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0) * context.sigmaZ(1)),
                  (std::pair<uint64_t, uint64_t>{0x000000000000000d, 0x000000000000000d}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1) * context.sigmaZ(2)),
                  (std::pair<uint64_t, uint64_t>{0x000000000000000d, 0x0000000000000034}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(2) * context.sigmaZ(3)),
                  (std::pair<uint64_t, uint64_t>{0x000000000000000d, 0x00000000000000d0}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(3) * context.sigmaZ(4)),
                  (std::pair<uint64_t, uint64_t>{0x000000000000000d, 0x0000000000000340}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(4) * context.sigmaZ(0)),
                  (std::pair<uint64_t, uint64_t>{0x000000000000000d, 0x0000000000000103}));

    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalHash_ChainMedium) {
        const size_t chain_length = 40;
        PauliContext context{chain_length, true, true};
        SiteHasher<2> hasher{context};

        // Canonical results:
        const SiteHasher<2>::Datum expected_single_hash{1, 0};
        const SiteHasher<2>::Datum expected_nn_hash{9, 0};
        ASSERT_EQ(hasher(context.sigmaX(0)), expected_single_hash);
        ASSERT_EQ(hasher(context.sigmaX(0) * context.sigmaY(1)), expected_nn_hash);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const auto shifted_single_hash = hasher(shifted_single_sequence);
            EXPECT_EQ(hasher.canonical_hash(shifted_single_sequence),
                      std::make_pair(expected_single_hash, shifted_single_hash))
                      << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence = context.sigmaX(base_index) * context.sigmaY((base_index+1) % chain_length);
            const auto shifted_nn_hash = hasher(shifted_nn_sequence);
            EXPECT_EQ(hasher.canonical_hash(shifted_nn_sequence),
                      std::make_pair(expected_nn_hash, shifted_nn_hash))
                      << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalHash_ChainLarger) {
        const size_t chain_length = 70;
        PauliContext context{chain_length, true, true};
        SiteHasher<3> hasher{context};

        // Canonical results:
        const SiteHasher<3>::Datum expected_single_hash{1, 0, 0};
        const SiteHasher<3>::Datum expected_nn_hash{9, 0, 0};
        ASSERT_EQ(hasher(context.sigmaX(0)), expected_single_hash);
        ASSERT_EQ(hasher(context.sigmaX(0) * context.sigmaY(1)), expected_nn_hash);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const auto shifted_single_hash = hasher(shifted_single_sequence);
            EXPECT_EQ(hasher.canonical_hash(shifted_single_sequence),
                      std::make_pair(expected_single_hash, shifted_single_hash))
                      << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence = context.sigmaX(base_index) * context.sigmaY((base_index+1) % chain_length);
            const auto shifted_nn_hash = hasher(shifted_nn_sequence);
            EXPECT_EQ(hasher.canonical_hash(shifted_nn_sequence),
                      std::make_pair(expected_nn_hash, shifted_nn_hash))
                      << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalHash_LatticeSmall) {
        PauliContext context{2, 2, true, true}; // 2x2 lattice
        SiteHasher<1> hasher{context};

        // Single qubits
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000001}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000004}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(2)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000010}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(3)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000001, 0x0000000000000040}));

        // X1Z2 vertical pair (prefers Z1X2...!)
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0, 0) * context.sigmaZ(1, 0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000007, 0x000000000000000d}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1, 0) * context.sigmaZ(0, 0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000007, 0x0000000000000007}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0, 1) * context.sigmaZ(1, 1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000007, 0x00000000000000d0}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1, 1) * context.sigmaZ(0, 1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000007, 0x0000000000000070}));

        // X1Z3 horizontal pair (prefers Z1X3...!)
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0, 0) * context.sigmaZ(0, 1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000013, 0x0000000000000031}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1, 0) * context.sigmaZ(1, 1)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000013, 0x00000000000000c4}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(0, 1) * context.sigmaZ(0, 0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000013, 0x0000000000000013}));
        EXPECT_EQ(hasher.canonical_hash(context.sigmaX(1, 1) * context.sigmaZ(1, 0)),
                  (std::pair<uint64_t, uint64_t>{0x0000000000000013, 0x000000000000004c}));
    }


    TEST(Scenarios_Pauli_SiteHasher, CanonicalSequence_ChainSmall) {
        const size_t chain_length = 5;
        PauliContext context{chain_length, true, true};
        SiteHasher<1> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single{hasher.canonical_sequence(shifted_single_sequence)};

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn{hasher.canonical_sequence(shifted_nn_sequence)};
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalSequence_ChainMedium) {
        const size_t chain_length = 40;
        PauliContext context{chain_length, true, true};
        SiteHasher<2> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single = hasher.canonical_sequence(shifted_single_sequence);

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn = hasher.canonical_sequence(shifted_nn_sequence);
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CanonicalSequence_ChainLarge) {
        const size_t chain_length = 72;
        PauliContext context{chain_length, true, true};
        SiteHasher<3> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single{hasher.canonical_sequence(shifted_single_sequence)};

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn{hasher.canonical_sequence(shifted_nn_sequence)};
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }
}