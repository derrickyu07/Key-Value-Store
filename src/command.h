#pragma once
#include <string>

enum class CommandType { Set, Get, Del, Expire, ExpireAt, Ttl, Exit, Compact, Invalid };

struct Command {
    CommandType type = CommandType::Invalid;
    std::string key;
    std::string value;
    long long number = 0;   // seconds for EXPIRE, milliseconds for EXPIREAT
};
Command parse(const std::string& line);