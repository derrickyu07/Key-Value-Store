#include <gtest/gtest.h>
#include "command.h"

TEST(ParseTest, ParsesSet) {
    Command cmd = parse("SET name derrick");
    EXPECT_EQ(cmd.type, CommandType::Set);
    EXPECT_EQ(cmd.key, "name");
    EXPECT_EQ(cmd.value, "derrick");
}

TEST(ParseTest, SetValueCanContainSpaces) {
    Command cmd = parse("SET greeting hello world");
    EXPECT_EQ(cmd.type, CommandType::Set);
    EXPECT_EQ(cmd.value, "hello world");
}

TEST(ParseTest, ParsesGet) {
    Command cmd = parse("GET name");
    EXPECT_EQ(cmd.type, CommandType::Get);
    EXPECT_EQ(cmd.key, "name");
}

TEST(ParseTest, ParsesDel) {
    Command cmd = parse("DEL name");
    EXPECT_EQ(cmd.type, CommandType::Del);
    EXPECT_EQ(cmd.key, "name");
}

TEST(ParseTest, ParsesExit) {
    EXPECT_EQ(parse("EXIT").type, CommandType::Exit);
}

TEST(ParseTest, SetWithoutValueIsInvalid) {
    EXPECT_EQ(parse("SET name").type, CommandType::Invalid);
}

TEST(ParseTest, GetWithoutKeyIsInvalid) {
    EXPECT_EQ(parse("GET").type, CommandType::Invalid);
}

TEST(ParseTest, UnknownCommandIsInvalid) {
    EXPECT_EQ(parse("FOO bar").type, CommandType::Invalid);
}

TEST(ParseTest, EmptyLineIsInvalid) {
    EXPECT_EQ(parse("").type, CommandType::Invalid);
}
TEST(ParseTest, ParsesCompact) {
    EXPECT_EQ(parse("COMPACT").type, CommandType::Compact);
}