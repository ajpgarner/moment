/**
 * operator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operator.h"

namespace NPATK::Tests {
    TEST(Operator, Party_Construct) {
        Party party{3};
        EXPECT_EQ(party.id, 3);
    }

    TEST(Operator, Party_CompareEqual) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_TRUE(pA == pA);
        EXPECT_TRUE(pA == pB);
        EXPECT_FALSE(pA == pC);

        EXPECT_TRUE(pB == pA);
        EXPECT_TRUE(pB == pB);
        EXPECT_FALSE(pB == pC);

        EXPECT_FALSE(pC == pA);
        EXPECT_FALSE(pC == pB);
        EXPECT_TRUE(pC == pC);
    }

    TEST(Operator, Party_CompareNotEqual) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_FALSE(pA != pA);
        EXPECT_FALSE(pA != pB);
        EXPECT_TRUE(pA != pC);

        EXPECT_FALSE(pB != pA);
        EXPECT_FALSE(pB != pB);
        EXPECT_TRUE(pB != pC);

        EXPECT_TRUE(pC != pA);
        EXPECT_TRUE(pC != pB);
        EXPECT_FALSE(pC != pC);
    }

    TEST(Operator, Party_CompareLess) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_FALSE(pA < pA);
        EXPECT_FALSE(pA < pB);
        EXPECT_TRUE(pA < pC);

        EXPECT_FALSE(pB < pA);
        EXPECT_FALSE(pB < pB);
        EXPECT_TRUE(pB < pC);

        EXPECT_FALSE(pC < pA);
        EXPECT_FALSE(pC < pB);
        EXPECT_FALSE(pC < pC);
    }

    TEST(Operator, Operator_Construct) {
        Operator test_op{13, Party{4}};
        EXPECT_EQ(test_op.id, 13);
        EXPECT_EQ(test_op.party, Party{4});
    }


    TEST(Operator, Operator_CompareEqual) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{14, Party{4}};

        EXPECT_TRUE(opA1 == opA2);
        EXPECT_TRUE(opA2 == opA1);
        EXPECT_FALSE(opA1 == opB);
        EXPECT_FALSE(opB == opA1);
        EXPECT_FALSE(opA1 == opC);
        EXPECT_FALSE(opC == opA1);

    }

    TEST(Operator, Operator_CompareNotEqual) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{14, Party{4}};

        EXPECT_FALSE(opA1 != opA2);
        EXPECT_FALSE(opA2 != opA1);
        EXPECT_TRUE(opA1 != opB);
        EXPECT_TRUE(opB != opA1);
        EXPECT_TRUE(opA1 != opC);
        EXPECT_TRUE(opC != opA1);
    }


    TEST(Operator, Operator_ComparePartyLess) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{12, Party{5}};

        Operator::PartyComparator comp{};

        EXPECT_FALSE(comp(opA1, opA1));
        EXPECT_FALSE(comp(opA1, opA2));
        EXPECT_TRUE(comp(opA1, opB));
        EXPECT_TRUE(comp(opA1, opC));

        EXPECT_FALSE(comp(opA2, opA1));
        EXPECT_FALSE(comp(opA2, opA2));
        EXPECT_TRUE(comp(opA2, opB));
        EXPECT_TRUE(comp(opA2, opC));

        EXPECT_FALSE(comp(opB, opA1));
        EXPECT_FALSE(comp(opB, opA2));
        EXPECT_FALSE(comp(opB, opB));
        EXPECT_FALSE(comp(opB, opC));

        EXPECT_FALSE(comp(opC, opA1));
        EXPECT_FALSE(comp(opC, opA2));
        EXPECT_FALSE(comp(opC, opB));
        EXPECT_FALSE(comp(opC, opC));
    }



    TEST(Operator, Operator_CompareRedundant) {
        Operator opA_idem{1, Party{1}, Operator::Flags::Idempotent};
        Operator opA_non{1, Party{1}, Operator::Flags::None};
        Operator opB{1, Party{1}, Operator::Flags::Idempotent};
        Operator opC{2, Party{1}, Operator::Flags::Idempotent};
        Operator opD{1, Party{2}, Operator::Flags::Idempotent};

        Operator::IsRedundant comp{};

        EXPECT_TRUE(comp(opA_idem, opA_idem));
        EXPECT_FALSE(comp(opA_non, opA_non));
        EXPECT_TRUE(comp(opB, opB));
        EXPECT_TRUE(comp(opC, opC));
        EXPECT_TRUE(comp(opD, opD));

        // Note: comp(opA_idem, opA_non) is undefined.

        EXPECT_TRUE(comp(opA_idem, opB));
        EXPECT_FALSE(comp(opA_idem, opC));
        EXPECT_FALSE(comp(opA_idem, opD));
        // EXPECT_FALSE(comp(opA_non, opB)); <- not defined, as symbols differ only by idempotency.
        EXPECT_FALSE(comp(opA_non, opC));
        EXPECT_FALSE(comp(opA_non, opD));

        EXPECT_TRUE(comp(opB, opA_idem));
        EXPECT_FALSE(comp(opC, opA_idem));
        EXPECT_FALSE(comp(opD, opA_idem));
        // EXPECT_FALSE(comp(opB, opA_non)); <- not defined, as symbols differ only by idempotency.
        EXPECT_FALSE(comp(opC, opA_non));
        EXPECT_FALSE(comp(opD, opA_non));
    }
}