/**
 * recursive_storage_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/recursive_index.h"

#include <sstream>

namespace Moment::Tests {

    namespace {
        class ChunkTest : public MonotonicChunkRecursiveStorage<size_t, ChunkTest> {
        public:
            explicit ChunkTest(std::span<const size_t> chunk_sizes, size_t max_depth, size_t zero = 0, ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage(chunk_sizes, max_depth, zero, offset) { }
            explicit ChunkTest(size_t zero = 0, ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage(zero, offset) { }
        };

        void set_and_read(ChunkTest& c, const std::initializer_list<const size_t> elems, size_t i, size_t expected_children) {
            std::stringstream ss;
            ss << "Elems: ";
            for (auto elem : elems) {
                ss << elem << ".";
            }

            c.set(elems, i);
            size_t x = c.access(elems);
            EXPECT_EQ(x, i) << ss.str();

            const auto& subtree = c.subtree(elems);
            EXPECT_EQ(subtree.access(), i) << ss.str();
            ASSERT_EQ(subtree.num_children(), expected_children) << ss.str();
        }

        void compare_result(const std::pair<size_t, std::vector<size_t>>& result,
                            size_t index, std::initializer_list<size_t> indices) {
            std::stringstream ss;
            ss << "Elems: ";
            for (auto elem : indices) {
                ss << elem << ".";
            }

            EXPECT_EQ(result.first, index) << ss.str();
            ASSERT_EQ(result.second.size(), indices.size())  << ss.str();
            size_t i = 0;
            for (auto exp_index : indices) {
                EXPECT_EQ(result.second[i], exp_index) << ss.str();
                ++i;
            }
        }
    }

    TEST(Utilities_RecursiveStorage, Chunk) {
        std::vector<size_t> chunk_sizes{2, 1, 3}; // 6 children.
        ChunkTest c{chunk_sizes, 3};
        size_t running_iter = 0;
        set_and_read(c, {}, running_iter++, 6); // []
        set_and_read(c, {0}, running_iter++, 4); // A
        set_and_read(c, {0, 2}, running_iter++, 3); // A B
        set_and_read(c, {0, 2, 3}, running_iter++, 0); // A B C
        set_and_read(c, {0, 2, 4}, running_iter++, 0); // A B C
        set_and_read(c, {0, 2, 5}, running_iter++, 0); // A B C
        set_and_read(c, {0, 3}, running_iter++, 0); // A C
        set_and_read(c, {0, 4}, running_iter++, 0); // A C
        set_and_read(c, {0, 5}, running_iter++, 0); // A C
        set_and_read(c, {1}, running_iter++, 4); // A
        set_and_read(c, {1, 2}, running_iter++, 3); // A B
        set_and_read(c, {1, 2, 3}, running_iter++, 0); // A B C
        set_and_read(c, {1, 2, 4}, running_iter++, 0); // A B C
        set_and_read(c, {1, 2, 5}, running_iter++, 0); // A B C
        set_and_read(c, {1, 3}, running_iter++, 0); // A C
        set_and_read(c, {1, 4}, running_iter++, 0); // A C
        set_and_read(c, {1, 5}, running_iter++, 0); // A C
        set_and_read(c, {2}, running_iter++, 3); // B
        set_and_read(c, {2, 3}, running_iter++, 0); // B C
        set_and_read(c, {2, 4}, running_iter++, 0); // B C
        set_and_read(c, {2, 5}, running_iter++, 0); // B C
        set_and_read(c, {3}, running_iter++, 0); // C
        set_and_read(c, {4}, running_iter++, 0); // C
        set_and_read(c, {5}, running_iter++, 0); // C
    }


    TEST(Utilities_RecursiveStorage, ChunkClipped) {
        std::vector<size_t> chunk_sizes{2, 1, 3}; // 6 children.
        ChunkTest c{chunk_sizes, 2};
        size_t running_iter = 0;
        set_and_read(c, {}, running_iter++, 6); // []
        set_and_read(c, {0}, running_iter++, 4); // A
        set_and_read(c, {0, 2}, running_iter++, 0); // A B
        set_and_read(c, {0, 3}, running_iter++, 0); // A C
        set_and_read(c, {0, 4}, running_iter++, 0); // A C
        set_and_read(c, {0, 5}, running_iter++, 0); // A C
        set_and_read(c, {1}, running_iter++, 4); // A
        set_and_read(c, {1, 2}, running_iter++, 0); // A B
        set_and_read(c, {1, 3}, running_iter++, 0); // A C
        set_and_read(c, {1, 4}, running_iter++, 0); // A C
        set_and_read(c, {1, 5}, running_iter++, 0); // A C
        set_and_read(c, {2}, running_iter++, 3); // B
        set_and_read(c, {2, 3}, running_iter++, 0); // B C
        set_and_read(c, {2, 4}, running_iter++, 0); // B C
        set_and_read(c, {2, 5}, running_iter++, 0); // B C
        set_and_read(c, {3}, running_iter++, 0); // C
        set_and_read(c, {4}, running_iter++, 0); // C
        set_and_read(c, {5}, running_iter++, 0); // C
    }

    TEST(Utilities_RecursiveStorage, ChunkVisitor) {
        std::vector<size_t> chunk_sizes{2, 1, 3};
        ChunkTest c{chunk_sizes, 3};

        // Try setting objects with visitor
        size_t sequential_set = 0;
        auto visitor_setter = [&](size_t& obj, const std::vector<size_t>& indices) {
            obj = sequential_set++;
        };
        c.visit(visitor_setter);

        // Try reading objects with visitor
        std::vector<std::pair<size_t, std::vector<size_t>>> results;
        auto visitor_checker = [&](const size_t& obj, const std::vector<size_t>& indices) {
            results.emplace_back(obj, indices);
        };
        c.visit(visitor_checker);

        ASSERT_EQ(results.size(), 24);
        compare_result(results[0], 0, {}); // []
        compare_result(results[1], 1, {0}); // A
        compare_result(results[2], 2, {0, 2}); // A B
        compare_result(results[3], 3, {0, 2, 3}); // A B C
        compare_result(results[4], 4, {0, 2, 4}); // A B C
        compare_result(results[5], 5, {0, 2, 5}); // A B C
        compare_result(results[6], 6, {0, 3}); // A C
        compare_result(results[7], 7, {0, 4}); // A C
        compare_result(results[8], 8, {0, 5}); // A C
        compare_result(results[9], 9,  {1}); // A
        compare_result(results[10], 10, {1, 2}); // A B
        compare_result(results[11], 11, {1, 2, 3}); // A B C
        compare_result(results[12], 12, {1, 2, 4}); // A B C
        compare_result(results[13], 13, {1, 2, 5}); // A B C
        compare_result(results[14], 14, {1, 3}); // A C
        compare_result(results[15], 15, {1, 4}); // A C
        compare_result(results[16], 16, {1, 5}); // A C
        compare_result(results[17], 17, {2}); // B
        compare_result(results[18], 18, {2, 3}); // A C
        compare_result(results[19], 19, {2, 4}); // A C
        compare_result(results[20], 20, {2, 5}); // A C
        compare_result(results[21], 21, {3}); // C
        compare_result(results[22], 22, {4}); // C
        compare_result(results[23], 23, {5}); // C
    }

    TEST(Utilities_RecursiveStorage, ChunkVisitorClipped) {
        std::vector<size_t> chunk_sizes{2, 1, 3};
        ChunkTest c{chunk_sizes, 2};

        // Try setting objects with visitor
        size_t sequential_set = 0;
        auto visitor_setter = [&](size_t& obj, const std::vector<size_t>& indices) {
            obj = sequential_set++;
        };
        c.visit(visitor_setter);

        // Try reading objects with visitor
        std::vector<std::pair<size_t, std::vector<size_t>>> results;
        auto visitor_checker = [&](const size_t& obj, const std::vector<size_t>& indices) {
            results.emplace_back(obj, indices);
        };
        c.visit(visitor_checker);

        ASSERT_EQ(results.size(), 18);
        compare_result(results[0], 0, {}); // []
        compare_result(results[1], 1, {0}); // A
        compare_result(results[2], 2, {0, 2}); // A B
        compare_result(results[3], 3, {0, 3}); // A C
        compare_result(results[4], 4, {0, 4}); // A C
        compare_result(results[5], 5, {0, 5}); // A C
        compare_result(results[6], 6,  {1}); // A
        compare_result(results[7], 7, {1, 2}); // A B
        compare_result(results[8], 8, {1, 3}); // A C
        compare_result(results[9], 9, {1, 4}); // A C
        compare_result(results[10], 10, {1, 5}); // A C
        compare_result(results[11], 11, {2}); // B
        compare_result(results[12], 12, {2, 3}); // A C
        compare_result(results[13], 13, {2, 4}); // A C
        compare_result(results[14], 14, {2, 5}); // A C
        compare_result(results[15], 15, {3}); // C
        compare_result(results[16], 16, {4}); // C
        compare_result(results[17], 17, {5}); // C
    }
}