#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

class KVStore {
public:
    explicit KVStore(std::size_t capacity = 1000);

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    std::size_t size() const;

private:
    struct Entry {
        std::string value;
        std::list<std::string>::iterator lruPos;  // where this key sits in lru_
    };

    void touch(Entry& entry);  // move key to front
    void evict();

    std::size_t capacity_;
    std::list<std::string> lru_;                     // front = most recently used
    std::unordered_map<std::string, Entry> map_;
};