#pragma once

#include <string>
#include "command.h"
#include "kv_store.h"
#include "wal.h"

class Database {
public:
    explicit Database(const std::string& logPath, std::size_t capacity = 1000);
    std::string execute(const Command& cmd);   // returns the text to print

private:
    KVStore store_;         // declared first, so it exists before replay runs
    WriteAheadLog log_;
};