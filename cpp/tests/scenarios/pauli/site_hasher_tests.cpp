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
        SiteHasher<1> hasher{0};
        EXPECT_EQ(sizeof(SiteHasher<1>::Datum), 8);
        EXPECT_EQ(SiteHasher<1>::qubits_per_slide, 32);

        EXPECT_EQ(hasher({}), 0);
    }

    TEST(Scenarios_Pauli_SiteHasher, Hash_MediumEmpty) {
        SiteHasher<2> hasher{0};
        EXPECT_EQ(sizeof(SiteHasher<2>::Datum), 16);
        EXPECT_EQ(SiteHasher<1>::qubits_per_slide, 32);
        EXPECT_EQ(hasher(std::vector<oper_name_t>{}), (std::array<uint64_t, 2>{0, 0}));
    }


    TEST(Scenarios_Pauli_SiteHasher, Hash_SmallChain5) {
        PauliContext context{5};
        SiteHasher<1> hasher{5};

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



    TEST(Scenarios_Pauli_SiteHasher, Hash_MediumChain5) {
        PauliContext context{5};
        SiteHasher<2> hasher{5};
        EXPECT_EQ(hasher(context.identity()),
                  (std::array<uint64_t, 2>{0x0000000000000000, 0}));
        EXPECT_EQ(hasher(context.sigmaX(0)),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0}));
        EXPECT_EQ(hasher(context.sigmaY(0)),
                  (std::array<uint64_t, 2>{0x0000000000000002, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(0)),
                  (std::array<uint64_t, 2>{0x0000000000000003, 0}));

        EXPECT_EQ(hasher(context.sigmaX(1)),
                  (std::array<uint64_t, 2>{0x0000000000000004, 0}));
        EXPECT_EQ(hasher(context.sigmaY(1)),
                  (std::array<uint64_t, 2>{0x0000000000000008, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(1)),
                  (std::array<uint64_t, 2>{0x000000000000000c, 0}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(1))),
                  (std::array<uint64_t, 2>{0x0000000000000005, 0}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaY(1))),
                  (std::array<uint64_t, 2>{0x0000000000000009, 0}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaZ(1))),
                  (std::array<uint64_t, 2>{0x000000000000000d, 0}));
    }


    TEST(Scenarios_Pauli_SiteHasher, Hash_MediumChain40) {
        PauliContext context{40};
        SiteHasher<2> hasher{40};
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

    TEST(Scenarios_Pauli_SiteHasher, Unhash_SmallChain5) {
        PauliContext context{5};
        SiteHasher<1> hasher{5};

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

    TEST(Scenarios_Pauli_SiteHasher, Unhash_MediumChain40) {
        PauliContext context{40};
        SiteHasher<2> hasher{40};

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
        SiteHasher<1> hasher{32};
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
        SiteHasher<1> hasher{25};
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
        SiteHasher<2> hasher{64};
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
        SiteHasher<2> hasher{40}; // So, 8 qubits [16 bits] on second page
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
        SiteHasher<3> hasher{96}; // So, 16 qubits [32 bits] on final page
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
        SiteHasher<3> hasher{80}; // So, 16 qubits [32 bits] on final page
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
        PauliContext context{8, true, true, 4}; // 4x2 wrapping grid
        SiteHasher<1> hasher{8, 4};
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
        PauliContext context{20, true, true, 4}; // 4x5 wrapping grid
        SiteHasher<2> hasher{20, 4};
        ASSERT_EQ(hasher.column_height, 4);
        ASSERT_EQ(hasher.row_width, 5);

        for (size_t row_id = 0; row_id < 4; ++row_id) {
            for (size_t shift = 0; shift < 5; ++shift) {
                EXPECT_EQ(hasher.col_shift(hasher(context.sigmaX(row_id, 0)), shift),
                          hasher(context.sigmaX(row_id, shift % 5))) << "row = " << row_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, ColShift_Larger) {
        PauliContext context{20, true, true, 4}; // 4x5 wrapping grid
        SiteHasher<3> hasher{20, 4};
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
        PauliContext context{64, true, true, 8}; // 8x8 grid
        SiteHasher<2> hasher{64, 8};
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
        PauliContext context{50, true, true, 5}; // 10x5 grid
        SiteHasher<2> hasher{50, 5};
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
        PauliContext context{96, true, true, 8}; // 8x12 grid
        SiteHasher<3> hasher{96, 8};
        ASSERT_EQ(hasher.column_height, 8);
        ASSERT_EQ(hasher.row_width, 12);

        for (size_t col_id = 0; col_id < 12; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                << "col = " << col_id;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, ExtractColumn_LargerUnaligned) {
        PauliContext context{70, true, true, 5}; // 14x5 grid
        SiteHasher<3> hasher{70, 5};
        ASSERT_EQ(hasher.column_height, 5);
        ASSERT_EQ(hasher.row_width, 14);

        for (size_t col_id = 0; col_id < 14; ++col_id) {
            EXPECT_EQ(hasher.extract_column(hasher(context.sigmaX(0, col_id) * context.sigmaZ(4, col_id)), col_id),
                      hasher(context.sigmaX(0, 0) * context.sigmaZ(4, 0))[0])
                << "col = " << col_id;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, RowShift_Small) {
        PauliContext context{8, true, true, 4}; // 4x2 wrapping grid
        SiteHasher<1> hasher{8, 4};

        for (size_t row_id = 0; row_id < 4; ++row_id) {
            EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, 0)), 0),
                      hasher(context.sigmaX(row_id, 0))) << "row = " << row_id;
            EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, 0)), 1),
                      hasher(context.sigmaX((row_id+1)%4, 0))) << "row = " << row_id;
            EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, 1)), 1),
                      hasher(context.sigmaX((row_id+1)%4, 1))) << "row = " << row_id;
        }
    }


    TEST(Scenarios_Pauli_SiteHasher, RowShift_MediumUnaligned) {
        const size_t column_height = 12;
        const size_t column_count = 4;
        PauliContext context{48, true, true, column_height }; // 12x4 wrapping grid
        SiteHasher<2> hasher{48, column_height};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                          << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                          << "row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, RowShift_MediumAligned) {
        const size_t column_height = 8;
        const size_t column_count = 8;
        PauliContext context{64, true, true, column_height}; // 8x8 wrapping grid
        SiteHasher<2> hasher{64, column_height};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }


    TEST(Scenarios_Pauli_SiteHasher, RowShift_LargerAligned) {
        const size_t column_height = 8;
        const size_t column_count = 10;
        PauliContext context{80, true, true, column_height}; // 7x10 wrapping grid
        SiteHasher<3> hasher{80, column_height};
        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, RowShift_LargerUnaligned) {
        const size_t column_height = 7;
        const size_t column_count = 10;
        PauliContext context{70, true, true, column_height}; // 7x10 wrapping grid
        SiteHasher<3> hasher{70, column_height};

        for (size_t row_id = 0; row_id < column_height ; ++row_id) {
            for (size_t col_id = 0; col_id < column_count; ++col_id) {
                // Idempotent
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 0),
                          hasher(context.sigmaX(row_id, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;

                // Shift by 1
                EXPECT_EQ(hasher.row_shift(hasher(context.sigmaX(row_id, col_id)), 1),
                          hasher(context.sigmaX((row_id + 1) % column_height, col_id)))
                                    << "row = " << row_id << ", col = " << col_id;
            }
        }
    }
}