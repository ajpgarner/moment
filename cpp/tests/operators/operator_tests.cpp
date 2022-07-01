/**
 * operator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/operator.h"

namespace NPATK::Tests {

    TEST(Operator, Operator_Construct) {
        Operator test_op{13, 4};
        EXPECT_EQ(test_op.id, 13);
        EXPECT_EQ(test_op.party, 4);
    }


    TEST(Operator, Operator_CompareEqual) {
        Operator opA1{13, 4};
        Operator opA2{13, 4};
        Operator opB{13, 5};
        Operator opC{14, 4};

        EXPECT_TRUE(opA1 == opA2);
        EXPECT_TRUE(opA2 == opA1);
        EXPECT_FALSE(opA1 == opB);
        EXPECT_FALSE(opB == opA1);
        EXPECT_FALSE(opA1 == opC);
        EXPECT_FALSE(opC == opA1);
    }

    TEST(Operator, Operator_CompareNotEqual) {
        Operator opA1{13, 4};
        Operator opA2{13, 4};
        Operator opB{13, 5};
        Operator opC{14, 4};

        EXPECT_FALSE(opA1 != opA2);
        EXPECT_FALSE(opA2 != opA1);
        EXPECT_TRUE(opA1 != opB);
        EXPECT_TRUE(opB != opA1);
        EXPECT_TRUE(opA1 != opC);
        EXPECT_TRUE(opC != opA1);
    }


    TEST(Operator, Operator_ComparePartyLess) {
        Operator opA1{13, 4};
        Operator opA2{13, 4};
        Operator opB{13, 5};
        Operator opC{12, 5};

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
        Operator opA_idem{1, 1, Operator::Flags::Idempotent};
        Operator opA_non{1, 1, Operator::Flags::None};
        Operator opB{1, 1, Operator::Flags::Idempotent};
        Operator opC{2, 1, Operator::Flags::Idempotent};
        Operator opD{1, 2, Operator::Flags::Idempotent};

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