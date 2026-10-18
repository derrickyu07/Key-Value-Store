#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

class KVStore {
public:

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
    std::size_t size() const;

private:
    //struct Entry {
    //    std::string value;
    //    std::list<std::string>::iterator lruPos;  // where this key sits in lru_
    //};

    //void touch(Entry& entry, const std::string& key);  // move key to front
    //void evictIfFull();

    //std::size_t capacity_;
    //std::list<std::string> lru_;                     // front = most recently used
    std::unordered_map<std::string, std::string> map_;
};