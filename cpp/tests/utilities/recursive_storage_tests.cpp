/**
 * recursive_storage_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/recursive_index.h"

#include <sstream>

namespace NPATK::Tests {

    namespace {
        class ChunkTest : public MonotonicChunkRecursiveStorage<size_t, ChunkTest> {
        public:
            explicit ChunkTest(std::span<const size_t> chunk_sizes, size_t zero = 0, ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage(chunk_sizes, zero, offset) { }
            explicit ChunkTest(size_t zero = 0, ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage(zero, offset) { }
        };

        void set_and_read(ChunkTest& c, std::initializer_list<size_t> elems, size_t i, size_t expected_children) {
            std::stringstream ss;
            ss << "Elems: ";
            for (auto elem : elems) {
                ss << elem << ".";
            }

            c.set(elems, i);
            auto x = c.access(elems);
            EXPECT_EQ(x, i) << ss.str();

            const auto& subtree = c.subtree(elems);
            EXPECT_EQ(subtree.access(), i);
            ASSERT_EQ(subtree.num_children(), expected_children) << ss.str();
        }
    }

    TEST(RecursiveStorage, Chunk) {
        std::vector<size_t> chunk_sizes{2, 1, 3}; // 6 children.
        ChunkTest c{chunk_sizes};
        size_t running_iter = 0;
        set_and_read(c, {}, running_iter++, 6); // []
        set_and_read(c, {0}, running_iter++, 4); // A
        set_and_read(c, {1}, running_iter++, 4); // A
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
        set_and_read(c, {2, 3}, running_iter++, 0); // A C
        set_and_read(c, {2, 4}, running_iter++, 0); // A C
        set_and_read(c, {2, 5}, running_iter++, 0); // A C
        set_and_read(c, {3}, running_iter++, 0); // C
        set_and_read(c, {4}, running_iter++, 0); // C
        set_and_read(c, {5}, running_iter++, 0); // C
    }
}