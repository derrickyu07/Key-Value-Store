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

TEST(KVStoreLruTest, EvictsLeastRecentlyUsedWhenFull) {
    KVStore store(2);
    store.set("a", "1");
    store.set("b", "2");
    store.set("c", "3");   // full, so "a" is evicted

    EXPECT_FALSE(store.get("a").has_value());
    EXPECT_EQ(store.get("b").value_or(""), "2");
    EXPECT_EQ(store.get("c").value_or(""), "3");
    EXPECT_EQ(store.size(), 2u);
}

TEST(KVStoreLruTest, GetMarksKeyAsRecentlyUsed) {
    KVStore store(2);
    store.set("a", "1");
    store.set("b", "2");
    store.get("a");        // "a" is now most recent, so "b" is oldest
    store.set("c", "3");

    EXPECT_TRUE(store.get("a").has_value());
    EXPECT_FALSE(store.get("b").has_value());
}

TEST(KVStoreLruTest, OverwriteMarksKeyAsRecentlyUsed) {
    KVStore store(2);
    store.set("a", "1");
    store.set("b", "2");
    store.set("a", "updated");
    store.set("c", "3");

    EXPECT_EQ(store.get("a").value_or(""), "updated");
    EXPECT_FALSE(store.get("b").has_value());
}

TEST(KVStoreLruTest, OverwriteDoesNotEvict) {
    KVStore store(2);
    store.set("a", "1");
    store.set("b", "2");
    store.set("a", "updated");   // same key, so no new slot needed

    EXPECT_EQ(store.size(), 2u);
    EXPECT_TRUE(store.get("b").has_value());
}

TEST(KVStoreLruTest, DelFreesSpace) {
    KVStore store(2);
    store.set("a", "1");
    store.set("b", "2");
    store.del("a");
    store.set("c", "3");         // there's room now, so nothing is evicted

    EXPECT_TRUE(store.get("b").has_value());
    EXPECT_TRUE(store.get("c").has_value());
    EXPECT_EQ(store.size(), 2u);
}
TEST(KVStoreLruTest, EntriesListedLeastRecentFirst) {
    KVStore store;
    store.set("a", "1");
    store.set("b", "2");
    store.set("c", "3");
    store.get("a");   // order is now b, c, a

    auto entries = store.entries();
    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0].first, "b");
    EXPECT_EQ(entries[1].first, "c");
    EXPECT_EQ(entries[2].first, "a");
    EXPECT_EQ(entries[2].second, "1");
}