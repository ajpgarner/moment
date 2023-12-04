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

        EXPECT_EQ(hasher({}), (std::array<uint64_t, 1>{0}));
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

        EXPECT_EQ(hasher(context.identity().raw()), (std::array<uint64_t, 1>{0x0000000000000000}));
        EXPECT_EQ(hasher(context.sigmaX(0).raw()), (std::array<uint64_t, 1>{0x0000000000000001}));
        EXPECT_EQ(hasher(context.sigmaY(0).raw()), (std::array<uint64_t, 1>{0x0000000000000002}));
        EXPECT_EQ(hasher(context.sigmaZ(0).raw()), (std::array<uint64_t, 1>{0x0000000000000003}));

        EXPECT_EQ(hasher(context.sigmaX(1).raw()), (std::array<uint64_t, 1>{0x0000000000000004}));
        EXPECT_EQ(hasher(context.sigmaY(1).raw()), (std::array<uint64_t, 1>{0x0000000000000008}));
        EXPECT_EQ(hasher(context.sigmaZ(1).raw()), (std::array<uint64_t, 1>{0x000000000000000c}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(1)).raw()), (std::array<uint64_t, 1>{0x0000000000000005}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaY(1)).raw()), (std::array<uint64_t, 1>{0x0000000000000009}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaZ(1)).raw()), (std::array<uint64_t, 1>{0x000000000000000d}));
    }



    TEST(Scenarios_Pauli_SiteHasher, Hash_MediumChain5) {
        PauliContext context{5};
        SiteHasher<2> hasher{5};
        EXPECT_EQ(hasher(context.identity().raw()),
                  (std::array<uint64_t, 2>{0x0000000000000000, 0}));
        EXPECT_EQ(hasher(context.sigmaX(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0}));
        EXPECT_EQ(hasher(context.sigmaY(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000002, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000003, 0}));

        EXPECT_EQ(hasher(context.sigmaX(1).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000004, 0}));
        EXPECT_EQ(hasher(context.sigmaY(1).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000008, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(1).raw()),
                  (std::array<uint64_t, 2>{0x000000000000000c, 0}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(1)).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000005, 0}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaY(1)).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000009, 0}));
        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaZ(1)).raw()),
                  (std::array<uint64_t, 2>{0x000000000000000d, 0}));
    }


    TEST(Scenarios_Pauli_SiteHasher, Hash_MediumChain40) {
        PauliContext context{40};
        SiteHasher<2> hasher{40};
        ASSERT_EQ(hasher.qubits_per_slide, 32);

        EXPECT_EQ(hasher(context.identity().raw()),
                  (std::array<uint64_t, 2>{0x0000000000000000, 0}));

        EXPECT_EQ(hasher(context.sigmaX(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0}));
        EXPECT_EQ(hasher(context.sigmaY(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000002, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(0).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000003, 0}));


        EXPECT_EQ(hasher(context.sigmaX(16).raw()),
                  (std::array<uint64_t, 2>{0x0000000100000000, 0}));
        EXPECT_EQ(hasher(context.sigmaY(16).raw()),
                  (std::array<uint64_t, 2>{0x0000000200000000, 0}));
        EXPECT_EQ(hasher(context.sigmaZ(16).raw()),
                  (std::array<uint64_t, 2>{ 0x0000000300000000, 0}));


        EXPECT_EQ(hasher(context.sigmaX(32).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000001}));
        EXPECT_EQ(hasher(context.sigmaY(32).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000002}));
        EXPECT_EQ(hasher(context.sigmaZ(32).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000003}));


        EXPECT_EQ(hasher(context.sigmaX(33).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000004}));
        EXPECT_EQ(hasher(context.sigmaY(33).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000008}));
        EXPECT_EQ(hasher(context.sigmaZ(33).raw()),
                  (std::array<uint64_t, 2>{0, 0x000000000000000c}));

        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaX(33)).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000005}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaY(33)).raw()),
                  (std::array<uint64_t, 2>{0, 0x0000000000000009}));
        EXPECT_EQ(hasher((context.sigmaX(32) * context.sigmaZ(33)).raw()),
                  (std::array<uint64_t, 2>{0, 0x000000000000000d}));

        EXPECT_EQ(hasher((context.sigmaX(0) * context.sigmaX(33)).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000001, 0x0000000000000004}));
        EXPECT_EQ(hasher((context.sigmaX(1) * context.sigmaY(33)).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000004, 0x0000000000000008}));
        EXPECT_EQ(hasher((context.sigmaX(2) * context.sigmaZ(33)).raw()),
                  (std::array<uint64_t, 2>{0x0000000000000010, 0x000000000000000c}));
    }

    TEST(Scenarios_Pauli_SiteHasher, Unhash_SmallChain5) {
        PauliContext context{5};
        SiteHasher<1> hasher{5};

        // Single qubits
        for (size_t q = 0; q < 5; ++q) {
            // Sigma X
            auto sequenceX = context.sigmaX(q);
            auto hashX = hasher.hash(sequenceX.raw());
            OperatorSequence reconstructX{hasher.unhash(hashX), context};
            EXPECT_EQ(reconstructX, sequenceX) << "q = " << q;

            // Sigma Y
            auto sequenceY = context.sigmaY(q);
            auto hashY = hasher.hash(sequenceY.raw());
            OperatorSequence reconstructY{hasher.unhash(hashY), context};
            EXPECT_EQ(reconstructY, sequenceY) << "q = " << q;

            // Sigma Z
            auto sequenceZ = context.sigmaZ(q);
            auto hashZ = hasher.hash(sequenceZ.raw());
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
                auto hashXZ = hasher.hash(sequenceXZ.raw());
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
            auto hashX = hasher.hash(sequenceX.raw());
            OperatorSequence reconstructX{hasher.unhash(hashX), context};
            EXPECT_EQ(reconstructX, sequenceX) << "q = " << q;

            // Sigma Y
            auto sequenceY = context.sigmaY(q);
            auto hashY = hasher.hash(sequenceY.raw());
            OperatorSequence reconstructY{hasher.unhash(hashY), context};
            EXPECT_EQ(reconstructY, sequenceY) << "q = " << q;

            // Sigma Z
            auto sequenceZ = context.sigmaZ(q);
            auto hashZ = hasher.hash(sequenceZ.raw());
            OperatorSequence reconstructZ{hasher.unhash(hashZ), context};
            EXPECT_EQ(reconstructZ, sequenceZ) << "q = " << q;
        }

        // Pairs of qubits
        for (size_t q = 1; q < 40; ++q) {
            // Sigma X1 * Zq
            auto sequenceXZ = context.sigmaX(0) * context.sigmaZ(q);
            auto hashXZ = hasher.hash(sequenceXZ.raw());
            OperatorSequence reconstructXZ{hasher.unhash(hashXZ), context};
            EXPECT_EQ(reconstructXZ, sequenceXZ) << "q = " << q;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_SmallAligned) {
        PauliContext context{32};
        SiteHasher<1> hasher{32};
        EXPECT_EQ(hasher.mask, 0xffffffffffffffff);

        // Small shift
        for (size_t shift_index = 0; shift_index < 32; ++shift_index) {
            // One qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;

            // Two qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%32)).raw()))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_SmallUnaligned) {
        PauliContext context{25};
        SiteHasher<1> hasher{25};
        EXPECT_EQ(hasher.mask, 0x0003ffffffffffff);

        // Small shift
        for (size_t shift_index = 0; shift_index < 25; ++shift_index) {
            // One qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;

            // Two qubit
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%25)).raw()))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_MediumAligned) {
        PauliContext context{64};
        SiteHasher<2> hasher{64};
        EXPECT_EQ(hasher.mask, 0xffffffffffffffff);
        EXPECT_EQ(hasher.qubits_on_final_slide, 32);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%64)).raw()))
                                << " shift_index = " << shift_index;
        }

        // Trickier two qubit shifts
        for (size_t shift_index = 0; shift_index < 64; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(31)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+31)%64)).raw()))
                                << " shift_index = " << shift_index;
        }

    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_MediumUnaligned) {
        PauliContext context{40};
        SiteHasher<2> hasher{40}; // So, 8 qubits [16 bits] on second page
        ASSERT_EQ(hasher.mask, 0x000000000000ffff);
        ASSERT_EQ(hasher.qubits_on_final_slide, 8);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+3)%40)).raw()))
                      << " shift_index = " << shift_index;
        }

        // Trickier two qubit shifts
        for (size_t shift_index = 0; shift_index < 40; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(31)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index+31)%40)).raw()))
                      << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_LargerAligned) {
        PauliContext context{96};
        SiteHasher<3> hasher{96}; // So, 16 qubits [32 bits] on final page
        ASSERT_EQ(hasher.qubits_on_final_slide, 32);
        ASSERT_EQ(hasher.mask, 0xffffffffffffffff);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 96; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 96; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaZ(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaZ((shift_index + 3) % 96)).raw()))
                                << " shift_index = " << shift_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, CyclicShift_LargerUnaligned) {
        PauliContext context{80};
        SiteHasher<3> hasher{80}; // So, 16 qubits [32 bits] on final page
        ASSERT_EQ(hasher.qubits_on_final_slide, 16);
        ASSERT_EQ(hasher.mask, 0x00000000ffffffff);

        // One qubit shifts
        for (size_t shift_index = 0; shift_index < 80; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher(context.sigmaX(0).raw()), shift_index),
                      hasher(context.sigmaX(shift_index).raw())) << " shift_index = " << shift_index;
        }

        // Two qubit shifts
        for (size_t shift_index = 0; shift_index < 80; ++shift_index) {
            EXPECT_EQ(hasher.cyclic_shift(hasher((context.sigmaX(0) * context.sigmaY(3)).raw()), shift_index),
                      hasher((context.sigmaX(shift_index) * context.sigmaY((shift_index + 3) % 80)).raw()))
                                << " shift_index = " << shift_index;
        }
    }
}