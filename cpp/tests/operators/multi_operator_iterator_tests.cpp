/**
 * multi_operator_iterator_tests.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "dictionary/multi_operator_iterator.h"

#include "scenarios/locality/locality_context.h"

namespace Moment::Tests {
    using namespace Moment::Locality;
    namespace {
        void assert_current_raw_value(const std::string& label,
                                      const MultiOperatorIterator& iter,
                                      const MultiOperatorIterator& iter_end,
                                      const sequence_storage_t& expected_raw) {
            ASSERT_NE(iter, iter_end) << label;
            ASSERT_EQ(iter.raw().size(), expected_raw.size()) << label;
            for (size_t idx = 0; idx < expected_raw.size(); ++idx) {
                EXPECT_EQ(iter.raw()[idx], expected_raw[idx]) << label << ", index = " << idx;
            }
        }
    }

    TEST(Operators_MultiOperatorIterator, Construct_NoLength) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 0};
        EXPECT_EQ(iter, iter);
        EXPECT_EQ(iter, MultiOperatorIterator::end_of(collection, 0));
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthOne2x2) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 1};
        auto iter_end = MultiOperatorIterator::end_of(collection, 1);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 1);
        EXPECT_EQ(osA0[0], collection.Parties[0][0]);
        auto osRawA0 = iter.raw();
        EXPECT_EQ(osRawA0[0], collection.Parties[0][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 1);
        EXPECT_EQ(osA1[0], collection.Parties[0][1]);
        auto osRawA1 = iter.raw();
        EXPECT_EQ(osRawA1[0], collection.Parties[0][1]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 1);
        EXPECT_EQ(osB0[0], collection.Parties[1][0]);
        auto osRawB0 = iter.raw();
        EXPECT_EQ(osRawB0[0], collection.Parties[1][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 1);
        EXPECT_EQ(osB1[0], collection.Parties[1][1]);
        auto osRawB1 = iter.raw();
        EXPECT_EQ(osRawB1[0], collection.Parties[1][1]);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthTwo) {
        Context collection{2};

        MultiOperatorIterator iter{collection, 2};
        auto iter_end = MultiOperatorIterator::end_of(collection, 2);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 2);
        EXPECT_EQ(osA0[0], 0);
        EXPECT_EQ(osA0[1], 0);
        auto osRawA0 = iter.raw();
        EXPECT_EQ(osRawA0[0], 0);
        EXPECT_EQ(osRawA0[1], 0);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 2);
        EXPECT_EQ(osA1[0], 0);
        EXPECT_EQ(osA1[1], 1);
        auto osRawA1 = iter.raw();
        EXPECT_EQ(osRawA1[0], 0);
        EXPECT_EQ(osRawA1[1], 1);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 2);
        EXPECT_EQ(osB0[0], 1);
        EXPECT_EQ(osB0[1], 0);
        auto osRawB0 = iter.raw();
        EXPECT_EQ(osRawB0[0], 1);
        EXPECT_EQ(osRawB0[1], 0);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 2);
        EXPECT_EQ(osB1[0], 1);
        EXPECT_EQ(osB1[1], 1);
        auto osRawB1 = iter.raw();
        EXPECT_EQ(osRawB1[0], 1);
        EXPECT_EQ(osRawB1[1], 1);


        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthFour) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 4};
        auto iter_end = MultiOperatorIterator::end_of(collection, 4);
        // 4 * 4 * 4 * 4 = 256 combos
        for (size_t count = 0; count < 256; ++count) {
            ASSERT_NE(iter, iter_end);
            auto op_seq = *iter;
            EXPECT_FALSE(op_seq.empty());
            EXPECT_LE(op_seq.size(), 4);
            ++iter;
        }
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Operators_MultiOperatorIterator, Construct_LengthTwoOffset) {
        Context collection{4};

        MultiOperatorIterator iter{collection, 2, 2, 1};
        auto iter_end = MultiOperatorIterator::end_of(collection, 2);

        ASSERT_NE(iter, iter_end);
        auto v11 = *iter;
        ASSERT_EQ(v11.size(), 2);
        EXPECT_EQ(v11[0], 1);
        EXPECT_EQ(v11[1], 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        auto v12 = *iter;
        ASSERT_EQ(v12.size(), 2);
        EXPECT_EQ(v12[0], 1);
        EXPECT_EQ(v12[1], 2);

        ++iter;
        ASSERT_NE(iter, iter_end);
        auto v21 = *iter;
        ASSERT_EQ(v21.size(), 2);
        EXPECT_EQ(v21[0], 2);
        EXPECT_EQ(v21[1], 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        auto v22 = *iter;
        ASSERT_EQ(v22.size(), 2);
        EXPECT_EQ(v22[0], 2);
        EXPECT_EQ(v22[1], 2);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Operators_MultiOperatorIterator, CHSH_RawValues) {
        Locality::LocalityContext context{Locality::Party::MakeList(2, 2, 2)};
        ASSERT_EQ(context.size(), 4);

        MultiOperatorIterator iter_level0{context, 0};
        auto iter_level0_end = MultiOperatorIterator::end_of(context, 0);
        ASSERT_EQ(iter_level0, iter_level0_end);

        MultiOperatorIterator iter_level1{context, 1};
        auto iter_level1_end = MultiOperatorIterator::end_of(context, 1);

        assert_current_raw_value("L1 #0", iter_level1, iter_level1_end, sequence_storage_t{0});
        ++iter_level1;
        assert_current_raw_value("L1 #1", iter_level1, iter_level1_end, sequence_storage_t{1});
        ++iter_level1;
        assert_current_raw_value("L1 #2", iter_level1, iter_level1_end, sequence_storage_t{2});
        ++iter_level1;
        assert_current_raw_value("L1 #3", iter_level1, iter_level1_end, sequence_storage_t{3});
        ++iter_level1;
        ASSERT_EQ(iter_level1, iter_level1_end);

        MultiOperatorIterator iter_level2{context, 2};
        auto iter_level2_end = MultiOperatorIterator::end_of(context, 2);

        assert_current_raw_value("L2 #0", iter_level2, iter_level2_end, sequence_storage_t{0, 0});
        ++iter_level2;
        assert_current_raw_value("L2 #1", iter_level2, iter_level2_end, sequence_storage_t{0, 1});
        ++iter_level2;
        assert_current_raw_value("L2 #2", iter_level2, iter_level2_end, sequence_storage_t{0, 2});
        ++iter_level2;
        assert_current_raw_value("L2 #3", iter_level2, iter_level2_end, sequence_storage_t{0, 3});
        ++iter_level2;
        assert_current_raw_value("L2 #4", iter_level2, iter_level2_end, sequence_storage_t{1, 0});
        ++iter_level2;
        assert_current_raw_value("L2 #5", iter_level2, iter_level2_end, sequence_storage_t{1, 1});
        ++iter_level2;
        assert_current_raw_value("L2 #6", iter_level2, iter_level2_end, sequence_storage_t{1, 2});
        ++iter_level2;
        assert_current_raw_value("L2 #7", iter_level2, iter_level2_end, sequence_storage_t{1, 3});
        ++iter_level2;
        assert_current_raw_value("L2 #8", iter_level2, iter_level2_end, sequence_storage_t{2, 0});
        ++iter_level2;
        assert_current_raw_value("L2 #9", iter_level2, iter_level2_end, sequence_storage_t{2, 1});
        ++iter_level2;
        assert_current_raw_value("L2 #10", iter_level2, iter_level2_end, sequence_storage_t{2, 2});
        ++iter_level2;
        assert_current_raw_value("L2 #11", iter_level2, iter_level2_end, sequence_storage_t{2, 3});
        ++iter_level2;
        assert_current_raw_value("L2 #12", iter_level2, iter_level2_end, sequence_storage_t{3, 0});
        ++iter_level2;
        assert_current_raw_value("L2 #13", iter_level2, iter_level2_end, sequence_storage_t{3, 1});
        ++iter_level2;
        assert_current_raw_value("L2 #14", iter_level2, iter_level2_end, sequence_storage_t{3, 2});
        ++iter_level2;
        assert_current_raw_value("L2 #15", iter_level2, iter_level2_end, sequence_storage_t{3, 3});
        ++iter_level2;

        ASSERT_EQ(iter_level2, iter_level2_end);


    }


    TEST(Operators_MultiOperatorIterator, RangeTest) {
        Context collection{4};
        ASSERT_EQ(collection.size(), 4);

        size_t count = 0;
        for (auto opStr : MultiOperatorRange{collection, 4}) {
            size_t v[4];
            v[0] = (count >> 6) & 0x3;
            v[1] = (count >> 4) & 0x3;
            v[2] = (count >> 2) & 0x3;
            v[3] = count & 0x3;

            ASSERT_EQ(opStr.size(), 4);
            EXPECT_EQ(opStr[0], v[0]) << "count = " << count << ", v[0]=" << v[0];
            EXPECT_EQ(opStr[1], v[1]) << "count = " << count << ", v[1]=" << v[1];
            EXPECT_EQ(opStr[2], v[2]) << "count = " << count << ", v[2]=" << v[2];
            EXPECT_EQ(opStr[3], v[3]) << "count = " << count << ", v[3]=" << v[3];
            ++count;
        }
        EXPECT_EQ(count, 256);
    }


}