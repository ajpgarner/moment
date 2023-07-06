/**
 * iter_tuple_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "utilities/iter_tuple.h"

#include <vector>

namespace Moment::Tests {

    TEST(Utilities_IterTuple, EmptyVectors) {
        std::vector<int> vecInt{};
        std::vector<double> vecFloat{};

        IterTuple iter(vecInt.cbegin(), vecFloat.cbegin());

        EXPECT_EQ(std::get<0>(iter.iters), vecInt.cend());
        EXPECT_EQ(std::get<1>(iter.iters), vecFloat.cend());
    }

    TEST(Utilities_IterTuple, Comparison) {
        std::vector<int> vecInt{};
        std::vector<double> vecFloat{};

        IterTuple iter(vecInt.cbegin(), vecFloat.cbegin());
        IterTuple iterEnd(vecInt.cend(), vecFloat.cend());

        EXPECT_TRUE(iter == iterEnd);
        EXPECT_FALSE(iter != iterEnd);
    }

    TEST(Utilities_IterTuple, Dereference) {
        std::vector<int> vecInt{10};
        std::vector<double> vecFloat{20.0};

        IterTuple iter(vecInt.cbegin(), vecFloat.cbegin());

        auto [x, y] = *iter;
        EXPECT_EQ(x, 10);
        EXPECT_EQ(y, 20.0);

    }

    TEST(Utilities_IterTuple, Increment) {
        std::vector<int> vecInt{10, 13};
        std::vector<double> vecFloat{20.0, 25.0};

        IterTuple iter(vecInt.cbegin(), vecFloat.cbegin());
        const IterTuple iterEnd(vecInt.cend(), vecFloat.cend());

        auto [x, y] = *iter;
        EXPECT_EQ(x, 10);
        EXPECT_EQ(y, 20.0);
        ++iter;

        auto [w, z] = *iter;
        EXPECT_EQ(w, 13);
        EXPECT_EQ(z, 25.0);

        ++iter;
        EXPECT_EQ(iter, iterEnd);
    }

}