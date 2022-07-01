/**
 * persistent_storage_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/persistent_storage.h"

namespace NPATK::Tests {

    TEST(PersistentStorage, Signature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};
        EXPECT_EQ(strBank.signature, signature);
        EXPECT_EQ(strBank.count(), 0);
        EXPECT_TRUE(strBank.empty());
    }

    TEST(PersistentStorage, CheckSignature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};
        ASSERT_EQ(strBank.signature, signature);

        uint64_t good_sig = (static_cast<uint64_t>(signature)) << 32;
        uint64_t bad_sig = (static_cast<uint64_t>(signature)+1) << 32;
        EXPECT_TRUE(strBank.check_signature(good_sig));
        EXPECT_FALSE(strBank.check_signature(bad_sig));
    }

    TEST(PersistentStorage, SetAndRetrieveOnce) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg[] = "Hello world";
        auto item_to_store = std::make_unique<std::string>(msg);

        uint64_t item_id = strBank.store(std::move(item_to_store));
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.count(), 1);

        auto retrievedStr = strBank.get(item_id);
        ASSERT_TRUE(retrievedStr);
        EXPECT_EQ(*retrievedStr, msg);
    }

    TEST(PersistentStorage, SetAndRetrieveTwice) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg1[] = "Hello world";
        const char msg2[] = "A second string";
        auto item_to_store1 = std::make_unique<std::string>(msg1);
        auto item_to_store2 = std::make_shared<std::string>(msg2);

        uint64_t item_id1 = strBank.store(std::move(item_to_store1));
        uint64_t item_id2 = strBank.store(item_to_store2);
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.count(), 2);

        auto retrievedStr1 = strBank.get(item_id1);
        auto retrievedStr2 = strBank.get(item_id2);
        ASSERT_TRUE(retrievedStr1);
        ASSERT_TRUE(retrievedStr2);
        EXPECT_EQ(*retrievedStr1, msg1);
        EXPECT_EQ(*retrievedStr2, msg2);
    }

    TEST(PersistentStorage, SetAndRelease) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg[] = "Hello world";
        auto item_to_store = std::make_unique<std::string>(msg);

        uint64_t item_id = strBank.store(std::move(item_to_store));
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.count(), 1);

        strBank.release(item_id);
        EXPECT_TRUE(strBank.empty());
        EXPECT_EQ(strBank.count(), 0);
    }

    TEST(PersistentStorage, SetAndReleaseTwice) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        const char msg1[] = "Hello world";
        const char msg2[] = "A second string";
        auto item_to_store1 = std::make_unique<std::string>(msg1);
        auto item_to_store2 = std::make_shared<std::string>(msg2);

        uint64_t item_id1 = strBank.store(std::move(item_to_store1));
        uint64_t item_id2 = strBank.store(item_to_store2);
        ASSERT_FALSE(strBank.empty());
        ASSERT_EQ(strBank.count(), 2);

        strBank.release(item_id2);
        ASSERT_EQ(strBank.count(), 1);
        ASSERT_FALSE(strBank.empty());

        auto retrievedStr1 = strBank.get(item_id1);
        ASSERT_TRUE(retrievedStr1);
        EXPECT_EQ(*retrievedStr1, msg1);

        strBank.release(item_id1);
        EXPECT_TRUE(strBank.empty());
        EXPECT_EQ(strBank.count(), 0);
    }


    TEST(PersistentStorage, Error_BadSignature) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        uint64_t bad_id = (static_cast<uint64_t>(signature)+1) << 32;
        EXPECT_THROW(strBank.get(bad_id), bad_signature_error);
    }

    TEST(PersistentStorage, Error_BadID) {
        uint32_t signature = make_signature({'s','t','r','b'});
        PersistentStorage<std::string> strBank{signature};

        uint64_t bad_id = (static_cast<uint64_t>(signature)) << 32;
        EXPECT_THROW(strBank.get(bad_id), not_found_error);
    }
}