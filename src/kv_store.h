#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class KVStore {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;
    using NowFn = std::function<TimePoint()>;

    explicit KVStore(std::size_t capacity = 1000,
        NowFn now = [] { return Clock::now(); });

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    std::size_t size() const;

    bool expireAt(const std::string& key, TimePoint when);  // false if key missing
    long long ttlSeconds(const std::string& key);           // -2 missing, -1 no expiry
    std::optional<TimePoint> expiresAt(const std::string& key) const;
    TimePoint now() const { return now_(); }

    // Live entries, least recently used first. Expired keys are skipped.
    std::vector<std::pair<std::string, std::string>> entries() const;

private:
    struct Entry {
        std::string value;
        std::list<std::string>::iterator lruPos;
        std::optional<TimePoint> expiresAt;   // empty means no expiration
    };
    using MapIt = std::unordered_map<std::string, Entry>::iterator;

    MapIt findLive(const std::string& key);   // end() if missing or expired
    bool isExpired(const Entry& entry) const;
    void touch(Entry& entry);
    void evict();

    std::size_t capacity_;
    NowFn now_;
    std::list<std::string> lru_;
    std::unordered_map<std::string, Entry> map_;
};