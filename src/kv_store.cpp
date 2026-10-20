#include "kv_store.h"

KVStore::KVStore(std::size_t capacity, NowFn now):capacity_(capacity), now_(std::move(now)){

}

std::size_t KVStore::size() const{
	return map_.size();
}

void KVStore::set(const std::string& key, const std::string& value) {
	auto it = map_.find(key);
	if (it == map_.end()) {
		if (map_.size() == capacity_) {
			evict();
		}
		lru_.push_front(key);
		map_.emplace(key, Entry{ value ,lru_.begin()});
		return;
	}
	it->second.expiresAt.reset();
	it->second.value = value;
	touch(it->second);

}

bool KVStore::del(const std::string& key) {
	auto it = findLive(key);
	if (it != map_.end()) {
		lru_.erase(it->second.lruPos);
		map_.erase(it);
		return true;
	}
	return false;

}

std::optional<std::string> KVStore::get(const std::string& key) {
	auto it = findLive(key);
	if (it != map_.end()) {
		touch(it->second);
		return it->second.value;
	}
	return std::nullopt;
}

void KVStore::touch(Entry& entry) {
	lru_.splice(lru_.cbegin(), lru_, entry.lruPos);
}

void KVStore::evict() {
	map_.erase(lru_.back());
	lru_.pop_back();
}

std::vector<std::pair<std::string, std::string>> KVStore::entries() const {
	std::vector<std::pair<std::string,std::string>> result;
	result.reserve(map_.size());
	for (auto it = lru_.rbegin(); it != lru_.rend(); ++it) {
		const Entry& entry = map_.at(*it);
		if (isExpired(entry)) {
			continue;
		}
		result.emplace_back(*it, entry.value);
	}
	return result;
}

bool KVStore::isExpired(const Entry& entry) const {
	return entry.expiresAt && now_() >= *entry.expiresAt;
}

KVStore::MapIt KVStore::findLive(const std::string& key) {
	auto it = map_.find(key);
	if (it != map_.end() && isExpired(it->second)) {
		lru_.erase(it->second.lruPos);
		map_.erase(it);
		return map_.end();
	}
	return it;   // a live entry, or end() if the key was never there
}

 std::optional<KVStore::TimePoint> KVStore::expiresAt(const std::string& key) const {
    auto it = map_.find(key);
    if (it == map_.end()) {
        return std::nullopt;
    }
    return it->second.expiresAt;
}


bool KVStore::expireAt(const std::string& key, TimePoint when) {
	auto it = findLive(key);
	if (it == map_.end()) {
		return false;
	}
	it->second.expiresAt = when;
	return true;
}
long long KVStore::ttlSeconds(const std::string& key) {
	auto it = map_.find(key);
	if (it == map_.end()) {
		return -2;
	}
	if (!it->second.expiresAt) {
		return -1;
	}
	return std::chrono::duration_cast<std::chrono::seconds>(*it->second.expiresAt - now_()).count();
}