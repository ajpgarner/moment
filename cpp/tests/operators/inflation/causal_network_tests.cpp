/**
 * causal_network_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/inflation/causal_network.h"

namespace NPATK::Tests {

    TEST(CausalNetwork, Empty) {
        CausalNetwork ic{{}, {}};
        EXPECT_EQ(ic.Observables().size(), 0);
        EXPECT_EQ(ic.Sources().size(), 0);
    }

    TEST(CausalNetwork, Empty_NoSources) {
        CausalNetwork ic{{2, 2}, {}};
        EXPECT_EQ(ic.Observables().size(),2);
        EXPECT_EQ(ic.Sources().size(), 0);
    }

    TEST(CausalNetwork, Empty_NoObservables) {
        CausalNetwork ic{{}, {{},{},{}}};
        EXPECT_EQ(ic.Observables().size(), 0);
        EXPECT_EQ(ic.Sources().size(), 3);
    }

    TEST(CausalNetwork, Error_BadObservable) {
        EXPECT_THROW(CausalNetwork({0}, {{0}}), errors::bad_observable);
    }

    TEST(CausalNetwork, Error_BadSource) {
        EXPECT_THROW(CausalNetwork({2, 2}, {{1,2}}), errors::bad_source);
    }

    TEST(CausalNetwork, Construct_Line) {
        CausalNetwork ic{{2, 2}, {{0, 1}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].sources.contains(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].sources.contains(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));
    }

    TEST(CausalNetwork, Construct_Triangle) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 3);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].sources.size(), 2);
        EXPECT_TRUE(observables[0].sources.contains(0));
        EXPECT_TRUE(observables[0].sources.contains(2));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].sources.size(), 2);
        EXPECT_TRUE(observables[1].sources.contains(0));
        EXPECT_TRUE(observables[1].sources.contains(1));

        EXPECT_EQ(observables[2].id, 2);
        EXPECT_EQ(observables[2].outcomes, 2);
        EXPECT_EQ(observables[2].sources.size(), 2);
        EXPECT_TRUE(observables[2].sources.contains(1));
        EXPECT_TRUE(observables[2].sources.contains(2));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 3);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));

        EXPECT_EQ(sources[1].id, 1);
        EXPECT_EQ(sources[1].observables.size(), 2);
        EXPECT_TRUE(sources[1].observables.contains(1));
        EXPECT_TRUE(sources[1].observables.contains(2));

        EXPECT_EQ(sources[2].id, 2);
        EXPECT_EQ(sources[2].observables.size(), 2);
        EXPECT_TRUE(sources[2].observables.contains(0));
        EXPECT_TRUE(sources[2].observables.contains(2));
    }

    TEST(CausalNetwork, CountCopies_Pair) {
        CausalNetwork ic{{2, 3}, {{0, 1}}};
        ASSERT_EQ(ic.Observables().size(), 2);

        // Inflation level 1: A, B
        EXPECT_EQ(ic.Observables()[0].count_copies(1), 1);
        EXPECT_EQ(ic.Observables()[1].count_copies(1), 1);

        // Inflation level 2: A0, A1; etc.
        EXPECT_EQ(ic.Observables()[0].count_copies(2), 2);
        EXPECT_EQ(ic.Observables()[1].count_copies(2), 2);

        // Inflation level 3: A0, A1, A2; etc.
        EXPECT_EQ(ic.Observables()[0].count_copies(3), 3);
        EXPECT_EQ(ic.Observables()[1].count_copies(3), 3);
    }

    TEST(CausalNetwork, CountCopies_Triangle) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}};
        ASSERT_EQ(ic.Observables().size(), 3);

        // Inflation level 1: A, B, C
        EXPECT_EQ(ic.Observables()[0].count_copies(1), 1);
        EXPECT_EQ(ic.Observables()[1].count_copies(1), 1);
        EXPECT_EQ(ic.Observables()[2].count_copies(1), 1);

        // Inflation level 2: A00, A01, A10, A11; etc.
        EXPECT_EQ(ic.Observables()[0].count_copies(2), 4);
        EXPECT_EQ(ic.Observables()[1].count_copies(2), 4);
        EXPECT_EQ(ic.Observables()[2].count_copies(2), 4);

        // Inflation level 2: A000, A001, A010, etc...
        EXPECT_EQ(ic.Observables()[0].count_copies(3), 9);
        EXPECT_EQ(ic.Observables()[1].count_copies(3), 9);
        EXPECT_EQ(ic.Observables()[2].count_copies(3), 9);
    }

    TEST(CausalNetwork, CountOperators_Pair) {
        CausalNetwork ic{{2, 3}, {{0, 1}}};
        ASSERT_EQ(ic.Observables().size(), 2);

        // Inflation level 1:
        EXPECT_EQ(ic.Observables()[0].count_operators(1), 1);
        EXPECT_EQ(ic.Observables()[1].count_operators(1), 2);
        EXPECT_EQ(ic.total_operator_count(1), 3);

        // Inflation level 2:
        EXPECT_EQ(ic.Observables()[0].count_operators(2), 2);
        EXPECT_EQ(ic.Observables()[1].count_operators(2), 4);
        EXPECT_EQ(ic.total_operator_count(2), 6);

        // Inflation level 3:
        EXPECT_EQ(ic.Observables()[0].count_operators(3), 3);
        EXPECT_EQ(ic.Observables()[1].count_operators(3), 6);
        EXPECT_EQ(ic.total_operator_count(3), 9);
    }


    TEST(CausalNetwork, CountOperators_Triangle) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}};
        ASSERT_EQ(ic.Observables().size(), 3);

        // Inflation level 1: A, B, C
        EXPECT_EQ(ic.Observables()[0].count_operators(1), 1);
        EXPECT_EQ(ic.Observables()[1].count_operators(1), 1);
        EXPECT_EQ(ic.Observables()[2].count_operators(1), 1);
        EXPECT_EQ(ic.total_operator_count(1), 3);

        // Inflation level 2: A00, A01, A10, A11; etc.
        EXPECT_EQ(ic.Observables()[0].count_operators(2), 4);
        EXPECT_EQ(ic.Observables()[1].count_operators(2), 4);
        EXPECT_EQ(ic.Observables()[2].count_operators(2), 4);
        EXPECT_EQ(ic.total_operator_count(2), 12);

        // Inflation level 2: A000, A001, A010, etc...
        EXPECT_EQ(ic.Observables()[0].count_operators(3), 9);
        EXPECT_EQ(ic.Observables()[1].count_operators(3), 9);
        EXPECT_EQ(ic.Observables()[2].count_operators(3), 9);
        EXPECT_EQ(ic.total_operator_count(3), 27);
    }
}
