#include <gtest/gtest.h>
#include <filesystem>
#include "wal.h"

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