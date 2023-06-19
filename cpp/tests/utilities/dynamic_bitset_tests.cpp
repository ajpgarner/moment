/**
 * dynamic_bitset_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "utilities/dynamic_bitset.h"
#include "utilities/small_vector.h"

namespace Moment::Tests {
    TEST(Utilities_DynamicBitset, Empty_Empty) {
        DynamicBitset<uint64_t> bitset{0};
        EXPECT_EQ(bitset.bit_size, 0);
        EXPECT_EQ(bitset.page_count, 0);
        EXPECT_TRUE(bitset.empty());
        EXPECT_EQ(bitset.count(), 0);
        EXPECT_EQ(bitset.first_index(), bitset.bit_size);
    }

    TEST(Utilities_DynamicBitset, Empty_Small) {
        DynamicBitset<uint64_t> bitset{40};
        EXPECT_EQ(bitset.bit_size, 40);
        EXPECT_EQ(bitset.page_count, 1);
        for (size_t i = 0; i < 40; ++i) {
            EXPECT_FALSE(bitset.test(i)) << i;
        }
        EXPECT_TRUE(bitset.empty());
        EXPECT_EQ(bitset.count(), 0);
        EXPECT_EQ(bitset.first_index(), bitset.bit_size);
    }

    TEST(Utilities_DynamicBitset, Empty_Boundary) {
        DynamicBitset<uint64_t> bitset{64};
        EXPECT_EQ(bitset.bit_size, 64);
        EXPECT_EQ(bitset.page_count, 1);
        for (size_t i = 0; i < 64; ++i) {
            EXPECT_FALSE(bitset.test(i)) << i;
        }
        EXPECT_TRUE(bitset.empty());
        EXPECT_EQ(bitset.count(), 0);
        EXPECT_EQ(bitset.first_index(), bitset.bit_size);
    }

    TEST(Utilities_DynamicBitset, Empty_Large) {
        DynamicBitset<uint64_t> bitset{65};
        EXPECT_EQ(bitset.bit_size, 65);
        EXPECT_EQ(bitset.page_count, 2);
        for (size_t i = 0; i < 65; ++i) {
            EXPECT_FALSE(bitset.test(i)) << i;
        }
        EXPECT_TRUE(bitset.empty());
        EXPECT_EQ(bitset.count(), 0);
        EXPECT_EQ(bitset.first_index(), bitset.bit_size);
    }

    TEST(Utilities_DynamicBitset, Full_Small) {
        DynamicBitset<uint64_t> bitset{40, true};
        EXPECT_EQ(bitset.bit_size, 40);
        EXPECT_EQ(bitset.page_count, 1);
        for (size_t i = 0; i < 40; ++i) {
            EXPECT_TRUE(bitset.test(i)) << i;
        }
        EXPECT_FALSE(bitset.empty());
        EXPECT_EQ(bitset.count(), 40);
        EXPECT_EQ(bitset.first_index(), 0);
    }

    TEST(Utilities_DynamicBitset, Full_Boundary) {
        DynamicBitset<uint64_t> bitset{64, true};
        EXPECT_EQ(bitset.bit_size, 64);
        EXPECT_EQ(bitset.page_count, 1);
        for (size_t i = 0; i < 64; ++i) {
            EXPECT_TRUE(bitset.test(i)) << i;
        }
        EXPECT_FALSE(bitset.empty());
        EXPECT_EQ(bitset.count(), 64);
        EXPECT_EQ(bitset.first_index(), 0);
    }

    TEST(Utilities_DynamicBitset, Full_Large) {
        DynamicBitset<uint64_t> bitset{65, true};
        EXPECT_EQ(bitset.bit_size, 65);
        EXPECT_EQ(bitset.page_count, 2);
        for (size_t i = 0; i < 65; ++i) {
            EXPECT_TRUE(bitset.test(i)) << i;
        }
        EXPECT_FALSE(bitset.empty());
        EXPECT_EQ(bitset.count(), 65);
        EXPECT_EQ(bitset.first_index(),0 );
    }




    TEST(Utilities_DynamicBitset, SetTestUnset_Small) {
        for (size_t magic_bit = 0; magic_bit < 40; ++magic_bit) {
            DynamicBitset<uint64_t> bitset{40};
            EXPECT_TRUE(bitset.empty());
            bitset.set(magic_bit);
            EXPECT_EQ(bitset.count(), 1);
            EXPECT_FALSE(bitset.empty());
            EXPECT_EQ(bitset.first_index(), magic_bit);
            for (size_t i = 0; i < 40; ++i) {
                if (i == magic_bit) {
                    EXPECT_TRUE(bitset.test(i)) << i;
                } else {
                    EXPECT_FALSE(bitset.test(i)) << i;
                }
            }
            bitset.unset(magic_bit);
            EXPECT_TRUE(bitset.empty());
            for (size_t i = 0; i < 40; ++i) {
                EXPECT_FALSE(bitset.test(i)) << i;
            }
        }
    }

    TEST(Utilities_DynamicBitset, SetTestUnset_Exact) {
        for (size_t magic_bit = 0; magic_bit < 64; ++magic_bit) {
            DynamicBitset<uint64_t> bitset{64};
            EXPECT_TRUE(bitset.empty());
            bitset.set(magic_bit);
            EXPECT_EQ(bitset.count(), 1);
            EXPECT_FALSE(bitset.empty());
            EXPECT_EQ(bitset.first_index(), magic_bit);
            for (size_t i = 0; i < 64; ++i) {
                if (i == magic_bit) {
                    EXPECT_TRUE(bitset.test(i)) << i;
                } else {
                    EXPECT_FALSE(bitset.test(i)) << i;
                }
            }
            bitset.unset(magic_bit);
            EXPECT_TRUE(bitset.empty());
            for (size_t i = 0; i < 64; ++i) {
                EXPECT_FALSE(bitset.test(i)) << i;
            }
        }
    }

    TEST(Utilities_DynamicBitset, SetTestUnset_Large) {
        for (size_t magic_bit = 0; magic_bit < 70; ++magic_bit) {
            DynamicBitset<uint64_t> bitset{70};
            EXPECT_TRUE(bitset.empty());
            bitset.set(magic_bit);
            EXPECT_EQ(bitset.count(), 1);
            EXPECT_FALSE(bitset.empty());
            EXPECT_EQ(bitset.first_index(), magic_bit);
            for (size_t i = 0; i < 70; ++i) {
                if (i == magic_bit) {
                    EXPECT_TRUE(bitset.test(i)) << i;
                } else {
                    EXPECT_FALSE(bitset.test(i)) << i;
                }
            }
            bitset.unset(magic_bit);
            EXPECT_TRUE(bitset.empty());
            for (size_t i = 0; i < 70; ++i) {
                EXPECT_FALSE(bitset.test(i)) << i;
            }
        }
    }


    TEST(Utilities_DynamicBitset, SetTestUnset_SmallVectorSmall) {
        for (size_t magic_bit = 0; magic_bit < 40; ++magic_bit) {
            DynamicBitset<uint64_t, size_t, SmallVector<uint64_t, 1>> bitset{40};
            EXPECT_TRUE(bitset.empty());
            bitset.set(magic_bit);
            EXPECT_EQ(bitset.count(), 1);
            EXPECT_FALSE(bitset.empty());
            EXPECT_EQ(bitset.first_index(), magic_bit);
            for (size_t i = 0; i < 40; ++i) {
                if (i == magic_bit) {
                    EXPECT_TRUE(bitset.test(i)) << i;
                } else {
                    EXPECT_FALSE(bitset.test(i)) << i;
                }
            }
            bitset.unset(magic_bit);
            EXPECT_TRUE(bitset.empty());
            for (size_t i = 0; i < 40; ++i) {
                EXPECT_FALSE(bitset.test(i)) << i;
            }
        }
    }

    TEST(Utilities_DynamicBitset, SetTestUnset_SmallVectorLarge) {
        for (size_t magic_bit = 0; magic_bit < 70; ++magic_bit) {
            DynamicBitset<uint64_t, size_t, SmallVector<uint64_t, 1>> bitset{70};
            EXPECT_TRUE(bitset.empty());
            bitset.set(magic_bit);
            EXPECT_EQ(bitset.count(), 1);
            EXPECT_FALSE(bitset.empty());
            EXPECT_EQ(bitset.first_index(), magic_bit);
            for (size_t i = 0; i < 70; ++i) {
                if (i == magic_bit) {
                    EXPECT_TRUE(bitset.test(i)) << i;
                } else {
                    EXPECT_FALSE(bitset.test(i)) << i;
                }
            }
            bitset.unset(magic_bit);
            EXPECT_TRUE(bitset.empty());
            for (size_t i = 0; i < 70; ++i) {
                EXPECT_FALSE(bitset.test(i)) << i;
            }
        }
    }

    TEST(Utilities_DynamicBitset, ProxySet) {
        DynamicBitset<uint64_t> bitset{40};
        EXPECT_EQ(bitset.count(), 0);
        EXPECT_TRUE(bitset.empty());
        bitset[13] = true;
        EXPECT_EQ(bitset.count(), 1);
        EXPECT_FALSE(bitset.empty());
        EXPECT_TRUE(bitset.test(13));
        EXPECT_TRUE(bitset[13]);
    }



    TEST(Utilities_DynamicBitset, LogicalAND) {
        DynamicBitset<uint64_t> bitsetA{70};
        bitsetA.set(5);
        bitsetA.set(20);
        bitsetA.set(67);
        bitsetA.set(68);

        DynamicBitset<uint64_t> bitsetB{70};
        bitsetB.set(6);
        bitsetB.set(20);
        bitsetB.set(67);
        bitsetB.set(69);

        auto combo = bitsetA & bitsetB;
        ASSERT_EQ(combo.bit_size, 70);
        std::set<uint64_t> true_nums{20, 67};
        EXPECT_EQ(combo.count(), true_nums.size());
        for (size_t test = 0; test < 70; ++test) {
            if (true_nums.contains(test)) {
                EXPECT_TRUE(combo.test(test)) << test;
            } else {
                EXPECT_FALSE(combo.test(test)) << test;
            }
        }
    }

    TEST(Utilities_DynamicBitset, LogicalOR) {
        DynamicBitset<uint64_t> bitsetA{70};
        bitsetA.set(5);
        bitsetA.set(20);
        bitsetA.set(67);
        bitsetA.set(68);

        DynamicBitset<uint64_t> bitsetB{70};
        bitsetB.set(6);
        bitsetB.set(20);
        bitsetB.set(67);
        bitsetB.set(69);

        auto combo = bitsetA | bitsetB;
        ASSERT_EQ(combo.bit_size, 70);
        ASSERT_EQ(combo.bit_size, 70);
        std::set<uint64_t> true_nums{5, 6, 20, 67, 68, 69};
        EXPECT_EQ(combo.count(), true_nums.size());
        for (size_t test = 0; test < 70; ++test) {
            if (true_nums.contains(test)) {
                EXPECT_TRUE(combo.test(test)) << test;
            } else {
                EXPECT_FALSE(combo.test(test)) << test;
            }
        }
    }

    TEST(Utilities_DynamicBitset, LogicalNOT) {
        DynamicBitset<uint64_t> bitsetA{70};
        bitsetA.set(5);
        bitsetA.set(20);
        bitsetA.set(67);

        auto bitsetB = ~bitsetA;

        ASSERT_EQ(bitsetB.bit_size, 70);
        std::set<uint64_t> false_nums{5, 20, 67};
        EXPECT_EQ(bitsetB.count(), 67);
        for (size_t test = 0; test < 70; ++test) {
            if (false_nums.contains(test)) {
                EXPECT_FALSE(bitsetB.test(test)) << test;
            } else {
                EXPECT_TRUE(bitsetB.test(test)) << test;
            }
        }
    }

    TEST(Utilities_DynamicBitset, LogicalNOT_InPlace) {
        DynamicBitset<uint64_t> bitsetA{70};
        bitsetA.set(5);
        bitsetA.set(20);
        bitsetA.set(67);

        bitsetA.invert_in_place();

        ASSERT_EQ(bitsetA.bit_size, 70);
        std::set<uint64_t> false_nums{5, 20, 67};
        EXPECT_EQ(bitsetA.count(), 67);
        for (size_t test = 0; test < 70; ++test) {
            if (false_nums.contains(test)) {
                EXPECT_FALSE(bitsetA.test(test)) << test;
            } else {
                EXPECT_TRUE(bitsetA.test(test)) << test;
            }
        }
    }


    TEST(Utilities_DynamicBitset, Iterator_Small) {
        DynamicBitset<uint64_t> bitset{50};
        bitset.set(5);
        bitset.set(20);
        bitset.set(47);
        bitset.set(48);

        auto iter = bitset.begin();
        auto iter_end = bitset.end();
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 5);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 20);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 47);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 48);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Utilities_DynamicBitset, Iterator_Large) {
        DynamicBitset<uint64_t> bitset{70};
        bitset.set(5);
        bitset.set(20);
        bitset.set(47);
        bitset.set(48);
        bitset.set(64);
        bitset.set(65);
        bitset.set(68);

        auto iter = bitset.begin();
        auto iter_end = bitset.end();
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 5);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 20);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 47);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 48);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 64);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 65);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 68);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Utilities_DynamicBitset, ToSet) {
        DynamicBitset<uint32_t> bitset{70};
        bitset.set(5);
        bitset.set(20);
        bitset.set(47);
        bitset.set(48);
        bitset.set(64);
        bitset.set(65);
        bitset.set(68);
        ASSERT_EQ(bitset.count(), 7);

        auto set = bitset.to_set<int>();
        EXPECT_EQ(set.size(), 7);
        EXPECT_TRUE(set.contains(5));
        EXPECT_TRUE(set.contains(20));
        EXPECT_TRUE(set.contains(47));
        EXPECT_TRUE(set.contains(48));
        EXPECT_TRUE(set.contains(64));
        EXPECT_TRUE(set.contains(65));
        EXPECT_TRUE(set.contains(68));

    }
}
