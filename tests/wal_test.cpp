#include <gtest/gtest.h>
#include <filesystem>
#include "wal.h"
#include <fstream>

class WalTest : public ::testing::Test {
protected:
    std::string path =
        (std::filesystem::temp_directory_path() / "kvstore_wal_test.log").string();

    void SetUp() override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }
};

TEST_F(WalTest, ReplayOnMissingFileGivesEmptyStore) {
    WriteAheadLog log(path);
    KVStore store;
    log.replay(store);
    EXPECT_EQ(store.size(), 0u);
}

TEST_F(WalTest, ReplayRestoresSets) {
    {
        WriteAheadLog log(path);
        log.appendSet("name", "derrick");
        log.appendSet("city", "albany");
    }   // log is destroyed here, which closes the file

    WriteAheadLog log(path);
    KVStore store;
    log.replay(store);
    EXPECT_EQ(store.get("name").value_or(""), "derrick");
    EXPECT_EQ(store.get("city").value_or(""), "albany");
}

TEST_F(WalTest, ReplayAppliesDeletes) {
    {
        WriteAheadLog log(path);
        log.appendSet("name", "derrick");
        log.appendDel("name");
    }

    WriteAheadLog log(path);
    KVStore store;
    log.replay(store);
    EXPECT_FALSE(store.get("name").has_value());
}

TEST_F(WalTest, ReplayKeepsLatestValue) {
    {
        WriteAheadLog log(path);
        log.appendSet("city", "atlanta");
        log.appendSet("city", "albany");
    }

    WriteAheadLog log(path);
    KVStore store;
    log.replay(store);
    EXPECT_EQ(store.get("city").value_or(""), "albany");
}

TEST_F(WalTest, ValuesWithSpacesSurviveReplay) {
    {
        WriteAheadLog log(path);
        log.appendSet("greeting", "hello world");
    }

    WriteAheadLog log(path);
    KVStore store;
    log.replay(store);
    EXPECT_EQ(store.get("greeting").value_or(""), "hello world");
}

static std::size_t countLines(const std::string& path) {
    std::ifstream in(path);
    std::string line;
    std::size_t count = 0;
    while (std::getline(in, line)) ++count;
    return count;
}

TEST_F(WalTest, CompactKeepsOnlyLiveKeys) {
    KVStore store;
    {
        WriteAheadLog log(path);
        log.appendSet("a", "1");
        log.appendSet("a", "2");
        log.appendSet("b", "x");
        log.appendDel("b");
        log.appendSet("c", "3");
        log.replay(store);      // store now holds a=2, c=3
        log.compact(store);
    }

    EXPECT_EQ(countLines(path), 2u);

    KVStore restored;
    WriteAheadLog log(path);
    log.replay(restored);
    EXPECT_EQ(restored.get("a").value_or(""), "2");
    EXPECT_EQ(restored.get("c").value_or(""), "3");
    EXPECT_FALSE(restored.get("b").has_value());
}

TEST_F(WalTest, CompactPreservesRecencyOrder) {
    KVStore store(2);
    {
        WriteAheadLog log(path);
        log.appendSet("a", "1");
        log.appendSet("b", "2");
        log.replay(store);
        store.get("a");         // "a" is now most recent
        log.compact(store);
    }

    KVStore restored(2);
    WriteAheadLog log(path);
    log.replay(restored);
    restored.set("c", "3");     // should evict "b", not "a"

    EXPECT_TRUE(restored.get("a").has_value());
    EXPECT_FALSE(restored.get("b").has_value());
}

TEST_F(WalTest, AppendsAfterCompactStillWork) {
    KVStore store;
    {
        WriteAheadLog log(path);
        log.appendSet("a", "1");
        log.replay(store);
        log.compact(store);
        log.appendSet("b", "2");   // log must still be writable
    }

    KVStore restored;
    WriteAheadLog log(path);
    log.replay(restored);
    EXPECT_TRUE(restored.get("a").has_value());
    EXPECT_TRUE(restored.get("b").has_value());
}