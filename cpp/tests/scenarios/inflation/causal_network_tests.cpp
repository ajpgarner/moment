/**
 * causal_network_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/causal_network.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_CausalNetwork, Empty) {
        CausalNetwork ic{{}, {}};
        EXPECT_EQ(ic.Observables().size(), 0);
        EXPECT_EQ(ic.Sources().size(), 0);

        EXPECT_EQ(ic.implicit_source_count(), 0);
        EXPECT_EQ(ic.explicit_source_count(), 0);
    }

    TEST(Scenarios_Inflation_CausalNetwork, Empty_NoSources) {
        CausalNetwork ic{{2, 2}, {}};
        EXPECT_EQ(ic.Observables().size(),2);
        ASSERT_EQ(ic.Sources().size(), 2);
        const auto& sources = ic.Sources();
        EXPECT_EQ(sources[0].implicit, true);
        EXPECT_EQ(sources[0].observables.size(), 1);
        EXPECT_TRUE(sources[0].observables.contains(0));

        EXPECT_EQ(sources[1].implicit, true);
        EXPECT_EQ(sources[1].observables.size(), 1);
        EXPECT_TRUE(sources[1].observables.contains(1));


        EXPECT_EQ(ic.implicit_source_count(), 2);
        EXPECT_EQ(ic.explicit_source_count(), 0);


    }

    TEST(Scenarios_Inflation_CausalNetwork, Empty_NoObservables) {
        CausalNetwork ic{{}, {{},{},{}}};
        EXPECT_EQ(ic.Observables().size(), 0);
        EXPECT_EQ(ic.Sources().size(), 3);

        EXPECT_EQ(ic.implicit_source_count(), 0);
        EXPECT_EQ(ic.explicit_source_count(), 3);
    }

    TEST(Scenarios_Inflation_CausalNetwork, Error_BadSource) {
        EXPECT_THROW(CausalNetwork({2, 2}, {{1,2}}), errors::bad_source);
    }


    TEST(Scenarios_Inflation_CausalNetwork, Construct_Singleton) {
        CausalNetwork ic{{2}, {{0}}};
        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 1);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_TRUE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].contains_source(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 1);
        EXPECT_TRUE(sources[0].observables.contains(0));

        EXPECT_EQ(ic.implicit_source_count(), 0);
        EXPECT_EQ(ic.explicit_source_count(), 1);
    }

    TEST(Scenarios_Inflation_CausalNetwork, Construct_Line) {
        CausalNetwork ic{{2, 2}, {{0, 1}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_TRUE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].contains_source(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_TRUE(observables[1].projective());
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].contains_source(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));

        EXPECT_EQ(ic.implicit_source_count(), 0);
        EXPECT_EQ(ic.explicit_source_count(), 1);
    }

    TEST(Scenarios_Inflation_CausalNetwork, Construct_Triangle) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 3);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_TRUE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 2);
        EXPECT_TRUE(observables[0].contains_source(0));
        EXPECT_TRUE(observables[0].contains_source(2));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_TRUE(observables[1].projective());
        EXPECT_EQ(observables[1].sources.size(), 2);
        EXPECT_TRUE(observables[1].contains_source(0));
        EXPECT_TRUE(observables[1].contains_source(1));

        EXPECT_EQ(observables[2].id, 2);
        EXPECT_EQ(observables[2].outcomes, 2);
        EXPECT_EQ(observables[2].operators(), 1);
        EXPECT_TRUE(observables[2].projective());
        EXPECT_EQ(observables[2].sources.size(), 2);
        EXPECT_TRUE(observables[2].contains_source(1));
        EXPECT_TRUE(observables[2].contains_source(2));

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


        EXPECT_EQ(ic.implicit_source_count(), 0);
        EXPECT_EQ(ic.explicit_source_count(), 3);
    }


    TEST(Scenarios_Inflation_CausalNetwork, Construct_CVLine) {
        CausalNetwork ic{{0, 0}, {{0, 1}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 0);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_FALSE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].contains_source(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 0);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_FALSE(observables[1].projective());
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].contains_source(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));
    }

    TEST(Scenarios_Inflation_CausalNetwork, Construct_UnlinkedCVPair) {
        CausalNetwork ic{{0, 0}, {}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 0);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_FALSE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].contains_source(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 0);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_FALSE(observables[1].projective());
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].contains_source(1));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 2);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_TRUE(sources[0].implicit);
        EXPECT_EQ(sources[0].observables.size(), 1);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_EQ(sources[1].id, 1);
        EXPECT_TRUE(sources[1].implicit);
        EXPECT_EQ(sources[1].observables.size(), 1);
        EXPECT_TRUE(sources[1].observables.contains(1));


        EXPECT_EQ(ic.implicit_source_count(), 2);
        EXPECT_EQ(ic.explicit_source_count(), 0);
    }

    TEST(Scenarios_Inflation_CausalNetwork, Construct_LineAndSingleton) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}}};

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 3);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 2);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_TRUE(observables[0].projective());
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].contains_source(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_TRUE(observables[1].projective());
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].contains_source(0));

        EXPECT_EQ(observables[2].id, 2);
        EXPECT_EQ(observables[2].outcomes, 2);
        EXPECT_EQ(observables[2].operators(), 1);
        EXPECT_TRUE(observables[2].projective());
        EXPECT_EQ(observables[2].sources.size(), 1);
        EXPECT_TRUE(observables[2].contains_source(1));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 2);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_FALSE(sources[0].implicit);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));

        EXPECT_EQ(sources[1].id, 1);
        EXPECT_TRUE(sources[1].implicit);
        EXPECT_EQ(sources[1].observables.size(), 1);
        EXPECT_TRUE(sources[1].observables.contains(2));
    }

    TEST(Scenarios_Inflation_CausalNetwork, CountCopies_Pair) {
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

    TEST(Scenarios_Inflation_CausalNetwork, CountCopies_Triangle) {
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

    TEST(Scenarios_Inflation_CausalNetwork, CountScenarios_Pair) {
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


    TEST(Scenarios_Inflation_CausalNetwork, CountScenarios_Triangle) {
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

        // Inflation level 2: A01, A01, A02, A10, etc...
        EXPECT_EQ(ic.Observables()[0].count_operators(3), 9);
        EXPECT_EQ(ic.Observables()[1].count_operators(3), 9);
        EXPECT_EQ(ic.Observables()[2].count_operators(3), 9);
        EXPECT_EQ(ic.total_operator_count(3), 27);
    }

    TEST(Scenarios_Inflation_CausalNetwork, CountScenarios_LineSingleton) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}}};
        ASSERT_EQ(ic.Observables().size(), 3);

        // Inflation level 1: A, B, C
        EXPECT_EQ(ic.Observables()[0].count_operators(1), 1);
        EXPECT_EQ(ic.Observables()[1].count_operators(1), 1);
        EXPECT_EQ(ic.Observables()[2].count_operators(1), 1);
        EXPECT_EQ(ic.total_operator_count(1), 3);
        EXPECT_EQ(ic.total_source_count(1), 2);

        // Inflation level 2: A00, A01, A10, A11; etc.
        EXPECT_EQ(ic.Observables()[0].count_operators(2), 2);
        EXPECT_EQ(ic.Observables()[1].count_operators(2), 2);
        EXPECT_EQ(ic.Observables()[2].count_operators(2), 1);
        EXPECT_EQ(ic.total_operator_count(2), 5);
        EXPECT_EQ(ic.total_source_count(2), 3);

        // Inflation level 2: A01, A01, A02, A10, etc...
        EXPECT_EQ(ic.Observables()[0].count_operators(3), 3);
        EXPECT_EQ(ic.Observables()[1].count_operators(3), 3);
        EXPECT_EQ(ic.Observables()[2].count_operators(3), 1);
        EXPECT_EQ(ic.total_operator_count(3), 7);
        EXPECT_EQ(ic.total_source_count(3), 4);
    }

    TEST(Scenarios_Inflation_CausalNetwork, CountSources_Implicit) {
        CausalNetwork empty{{2, 2}, {}};

        ASSERT_EQ(empty.implicit_source_count(), 2);
        // Inflation 1
        EXPECT_EQ(empty.source_variant_to_global_source(1, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(1, 1, 0), 1);
        EXPECT_EQ(empty.global_source_to_source_variant(1, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(1, 1), (std::pair<size_t, size_t>{1, 0}));

        // Inflation 2
        EXPECT_EQ(empty.source_variant_to_global_source(2, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 1, 0), 1);
        EXPECT_EQ(empty.global_source_to_source_variant(2, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 1), (std::pair<size_t, size_t>{1, 0}));
    }

    TEST(Scenarios_Inflation_CausalNetwork, CountSources_Explicit) {
        CausalNetwork empty{{2, 2}, {{0}, {1}}};

        ASSERT_EQ(empty.explicit_source_count(), 2);

        // Inflation 1
        EXPECT_EQ(empty.source_variant_to_global_source(1, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(1, 1, 0), 1);
        EXPECT_EQ(empty.global_source_to_source_variant(1, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(1, 1), (std::pair<size_t, size_t>{1, 0}));

        // Inflation 2
        EXPECT_EQ(empty.source_variant_to_global_source(2, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 0, 1), 1);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 1, 0), 2);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 1, 1), 3);
        EXPECT_EQ(empty.global_source_to_source_variant(2, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 1), (std::pair<size_t, size_t>{0, 1}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 2), (std::pair<size_t, size_t>{1, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 3), (std::pair<size_t, size_t>{1, 1}));
    }

    TEST(Scenarios_Inflation_CausalNetwork, CountSources_Mixed) {
        CausalNetwork empty{{2, 2, 2}, {{0, 1}}};

        ASSERT_EQ(empty.explicit_source_count(), 1);
        ASSERT_EQ(empty.implicit_source_count(), 1);

        // Inflation 1
        EXPECT_EQ(empty.source_variant_to_global_source(1, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(1, 1, 0), 1);
        EXPECT_EQ(empty.global_source_to_source_variant(1, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(1, 1), (std::pair<size_t, size_t>{1, 0}));

        // Inflation 2
        EXPECT_EQ(empty.source_variant_to_global_source(2, 0, 0), 0);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 0, 1), 1);
        EXPECT_EQ(empty.source_variant_to_global_source(2, 1, 0), 2);
        EXPECT_EQ(empty.global_source_to_source_variant(2, 0), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 1), (std::pair<size_t, size_t>{0, 1}));
        EXPECT_EQ(empty.global_source_to_source_variant(2, 2), (std::pair<size_t, size_t>{1, 0}));
    }


    TEST(Scenarios_Inflation_CausalNetwork, PermuteSourceIndices_Trivial) {
        CausalNetwork line{{2, 2}, {{0, 1}}};

        std::map<oper_name_t, oper_name_t> permutation{};

        const auto& src_names = line.Observables()[0].sources;
        ASSERT_EQ(src_names.size(), 1);

        std::vector<oper_name_t> indexA{0};
        auto permuted_indexA = line.permute_variant(2, src_names, permutation, indexA);
        EXPECT_EQ(permuted_indexA, (std::vector<oper_name_t>{0}));

        std::vector<oper_name_t> indexB{1};
        auto permuted_indexB = line.permute_variant(2, src_names, permutation, indexB);
        EXPECT_EQ(permuted_indexB, (std::vector<oper_name_t>{1}));
    }

    TEST(Scenarios_Inflation_CausalNetwork, PermuteSourceIndices_Swap) {
        CausalNetwork line{{2, 2}, {{0, 1}, {1}}};

        std::map<oper_name_t, oper_name_t> permutation;
        permutation.emplace(0, 1);
        permutation.emplace(1, 0);

        const auto& src_names = line.Observables()[1].sources;
        ASSERT_EQ(src_names.size(), 2);

        std::vector<oper_name_t> indexA{0, 0};
        auto permuted_indexA = line.permute_variant(2, src_names, permutation, indexA);
        EXPECT_EQ(permuted_indexA, (std::vector<oper_name_t>{1, 0}));

        std::vector<oper_name_t> indexB{1, 0};
        auto permuted_indexB = line.permute_variant(2, src_names, permutation, indexB);
        EXPECT_EQ(permuted_indexB, (std::vector<oper_name_t>{0, 0}));
    }



    TEST(Scenarios_Inflation_CausalNetwork, UnflattenIndices_Triangle) {
        CausalNetwork ic{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}};
        ASSERT_EQ(ic.Observables().size(), 3);
        const auto& observables = ic.Observables();

        // Inflation level 1: A, B, C; no copies.
        EXPECT_EQ(observables[0].unflatten_index(1, 0), (std::vector<oper_name_t>{0, 0}));
        EXPECT_EQ(observables[1].unflatten_index(1, 0), (std::vector<oper_name_t>{0, 0}));
        EXPECT_EQ(observables[2].unflatten_index(1, 0), (std::vector<oper_name_t>{0, 0}));

        // Inflation level 2: A00, A01, A10, A11; etc.
        for (size_t obs = 0; obs < 3; ++obs) {
            EXPECT_EQ(observables[obs].unflatten_index(2, 0), (std::vector<oper_name_t>{0, 0})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(2, 1), (std::vector<oper_name_t>{1, 0})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(2, 2), (std::vector<oper_name_t>{0, 1})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(2, 3), (std::vector<oper_name_t>{1, 1})) << "obs = " << obs;
        }

        // Inflation level 3: A00, A01, A02, A10...; etc.
        for (size_t obs = 0; obs < 3; ++obs) {
            EXPECT_EQ(observables[obs].unflatten_index(3, 0), (std::vector<oper_name_t>{0, 0})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 1), (std::vector<oper_name_t>{1, 0})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 2), (std::vector<oper_name_t>{2, 0})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 3), (std::vector<oper_name_t>{0, 1})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 4), (std::vector<oper_name_t>{1, 1})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 5), (std::vector<oper_name_t>{2, 1})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 6), (std::vector<oper_name_t>{0, 2})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 7), (std::vector<oper_name_t>{1, 2})) << "obs = " << obs;
            EXPECT_EQ(observables[obs].unflatten_index(3, 8), (std::vector<oper_name_t>{2, 2})) << "obs = " << obs;
        }
    }
}
