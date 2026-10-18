#pragma once

#include <fstream>
#include <string>
#include "kv_store.h"

class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& path);

    void appendSet(const std::string& key, const std::string& value);
    void appendDel(const std::string& key);
    void replay(KVStore& store) const;

private:
    std::string path_;
    std::ofstream out_;
};