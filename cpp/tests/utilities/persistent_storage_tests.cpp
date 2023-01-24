/**
 * persistent_storage_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/persistent_storage.h"

namespace Moment::Tests {

    TEST(Utilities_PersistentStorage, Signature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};
        EXPECT_EQ(strBank.signature, signature);
        EXPECT_EQ(strBank.size(), 0);
        EXPECT_TRUE(strBank.empty());
    }

    TEST(Utilities_PersistentStorage, CheckSignature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};
        ASSERT_EQ(strBank.signature, signature);

        uint64_t good_sig = (static_cast<uint64_t>(signature)) << 32;
        uint64_t bad_sig = (static_cast<uint64_t>(signature)+1) << 32;
        EXPECT_TRUE(strBank.check_signature(good_sig));
        EXPECT_FALSE(strBank.check_signature(bad_sig));
    }

    TEST(Utilities_PersistentStorage, SetAndRetrieveOnce) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg[] = "Hello world";
        auto item_to_store = std::make_unique<std::string>(msg);

        uint64_t item_id = strBank.store(std::move(item_to_store));
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.size(), 1);

        auto retrievedStr = strBank.get(item_id);
        ASSERT_TRUE(retrievedStr);
        EXPECT_EQ(*retrievedStr, msg);
    }

    TEST(Utilities_PersistentStorage, SetAndRetrieveTwice) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg1[] = "Hello world";
        const char msg2[] = "A second string";
        auto item_to_store1 = std::make_unique<std::string>(msg1);
        auto item_to_store2 = std::make_shared<std::string>(msg2);

        uint64_t item_id1 = strBank.store(std::move(item_to_store1));
        uint64_t item_id2 = strBank.store(item_to_store2);
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.size(), 2);

        auto retrievedStr1 = strBank.get(item_id1);
        auto retrievedStr2 = strBank.get(item_id2);
        ASSERT_TRUE(retrievedStr1);
        ASSERT_TRUE(retrievedStr2);
        EXPECT_EQ(*retrievedStr1, msg1);
        EXPECT_EQ(*retrievedStr2, msg2);
    }

    TEST(Utilities_PersistentStorage, SetAndRelease) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg[] = "Hello world";
        auto item_to_store = std::make_unique<std::string>(msg);

        uint64_t item_id = strBank.store(std::move(item_to_store));
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.size(), 1);

        strBank.release(item_id);
        EXPECT_TRUE(strBank.empty());
        EXPECT_EQ(strBank.size(), 0);
    }

    TEST(Utilities_PersistentStorage, SetAndReleaseTwice) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg1[] = "Hello world";
        const char msg2[] = "A second string";
        auto item_to_store1 = std::make_unique<std::string>(msg1);
        auto item_to_store2 = std::make_shared<std::string>(msg2);

        uint64_t item_id1 = strBank.store(std::move(item_to_store1));
        uint64_t item_id2 = strBank.store(item_to_store2);
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.size(), 2);

        strBank.release(item_id2);
        ASSERT_EQ(strBank.size(), 1);
        ASSERT_FALSE(strBank.empty());

        auto retrievedStr1 = strBank.get(item_id1);
        ASSERT_TRUE(retrievedStr1);
        EXPECT_EQ(*retrievedStr1, msg1);

        strBank.release(item_id1);
        EXPECT_TRUE(strBank.empty());
        EXPECT_EQ(strBank.size(), 0);
    }

    TEST(Utilities_PersistentStorage, Iteration) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};
        const char msg1[] = "Hello world";
        const char msg2[] = "A second string";
        const char msg3[] = "The third string";
        auto id1 = strBank.store(std::make_unique<std::string>(msg1));
        auto id2 = strBank.store(std::make_unique<std::string>(msg2));
        auto id3 = strBank.store(std::make_unique<std::string>(msg3));
        ASSERT_EQ(strBank.size(), 3);

        auto [fId, fPtr] = strBank.first();
        EXPECT_EQ(fId, 0);
        ASSERT_NE(fPtr, nullptr);
        EXPECT_EQ(*fPtr, msg1);

        auto [sId, sPtr] = strBank.next(fId);
        EXPECT_EQ(sId, 1);
        ASSERT_NE(sPtr, nullptr);
        EXPECT_EQ(*sPtr, msg2);

        auto [tId, tPtr] = strBank.next(sId);
        EXPECT_EQ(tId, 2);
        ASSERT_NE(tPtr, nullptr);
        EXPECT_EQ(*tPtr, msg3);

        auto [endId, endPtr] = strBank.next(tId);
        EXPECT_EQ(endId, uint32_t(0xFFFFFFFF));
        EXPECT_EQ(endPtr, nullptr);
    }

    TEST(Utilities_PersistentStorage, Monoid_Premade) {
        uint32_t signature = make_signature({'1','s','t','r'});
        PersistentStorageMonoid<std::string> strMonoid{signature, std::make_shared<std::string>("Hello")};
        EXPECT_FALSE(strMonoid.empty());
        auto get_obj = strMonoid.get();
        ASSERT_TRUE(static_cast<bool>(get_obj));
        EXPECT_EQ(*get_obj, "Hello");
    }

    TEST(Utilities_PersistentStorage, Monoid_Deferred) {
        uint32_t signature = make_signature({'1','s','t','r'});
        PersistentStorageMonoid<std::string> strMonoid{signature};
        EXPECT_TRUE(strMonoid.empty());
        auto early_get = strMonoid.get();
        EXPECT_FALSE(static_cast<bool>(early_get));

        auto emplaced_object = strMonoid.create_if_empty("Hello world");
        ASSERT_TRUE(static_cast<bool>(emplaced_object));
        EXPECT_EQ(*emplaced_object, "Hello world");

        auto second_creation = strMonoid.create_if_empty("No cheesecake");
        ASSERT_TRUE(static_cast<bool>(second_creation));
        EXPECT_EQ(emplaced_object, second_creation);
        EXPECT_EQ(*second_creation, "Hello world");
    }

    TEST(Utilities_PersistentStorage, Error_BadSignature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        uint64_t bad_id = (static_cast<uint64_t>(signature)+1) << 32;
        EXPECT_THROW(strBank.get(bad_id), bad_signature_error);
    }

    TEST(Utilities_PersistentStorage, Error_BadID) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        uint64_t bad_id = (static_cast<uint64_t>(signature)) << 32;
        EXPECT_THROW(strBank.get(bad_id), not_found_error);
    }
}