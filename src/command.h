#pragma once
#include <string>

enum class CommandType { Set, Get, Del, Exit, Invalid };

struct Command {
    CommandType type = CommandType::Invalid;
    std::string key;
    std::string value;   // only used by SET
};

Command parse(const std::string& line);