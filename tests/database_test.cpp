#include <gtest/gtest.h>
#include <filesystem>
#include "database.h"

class DatabaseTest : public ::testing::Test {
protected:
    std::string path =
        (std::filesystem::temp_directory_path() / "kvstore_db_test.log").string();

    void SetUp() override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }
};

TEST_F(DatabaseTest, SetThenGet) {
    Database db(path);
    EXPECT_EQ(db.execute(parse("SET name derrick")), "OK");
    EXPECT_EQ(db.execute(parse("GET name")), "derrick");
}

TEST_F(DatabaseTest, GetMissingKeyReturnsNil) {
    Database db(path);
    EXPECT_EQ(db.execute(parse("GET ghost")), "(nil)");
}

TEST_F(DatabaseTest, InvalidCommandReturnsError) {
    Database db(path);
    EXPECT_EQ(db.execute(parse("FOO bar")), "ERR invalid command");
}

TEST_F(DatabaseTest, DataSurvivesRestart) {
    {
        Database db(path);
        db.execute(parse("SET name derrick"));
        db.execute(parse("SET city albany"));
        db.execute(parse("DEL city"));
    }

    Database db(path);
    EXPECT_EQ(db.execute(parse("GET name")), "derrick");
    EXPECT_EQ(db.execute(parse("GET city")), "(nil)");
}