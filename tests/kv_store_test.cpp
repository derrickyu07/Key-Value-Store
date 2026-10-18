#include <gtest/gtest.h>
#include "kv_store.h"

TEST(KVStoreTest, GetMissingKeyReturnsNothing) {
	KVStore store;
	EXPECT_FALSE(store.get("missing").has_value());
}

TEST(KVStoreTest, SetThenGetReturnsValue) {
    KVStore store;
    store.set("name", "derrick");

    auto result = store.get("name");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "derrick");
}

TEST(KVStoreTest, SetOverwritesExistingValue) {
    KVStore store;
    store.set("city", "atlanta");
    store.set("city", "albany");

    EXPECT_EQ(*store.get("city"), "albany");
    EXPECT_EQ(store.size(), 1u);
}

TEST(KVStoreTest, DelRemovesKey) {
    KVStore store;
    store.set("name", "derrick");

    EXPECT_TRUE(store.del("name"));
    EXPECT_FALSE(store.get("name").has_value());
    EXPECT_EQ(store.size(), 0u);
}

TEST(KVStoreTest, DelMissingKeyReturnsFalse) {
    KVStore store;
    EXPECT_FALSE(store.del("ghost"));
}