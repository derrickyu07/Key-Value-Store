#include "kv_store.h"

KVStore::KVStore(std::size_t capacity):capacity_(capacity){
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
	it->second.value = value;
	touch(it->second);

}

bool KVStore::del(const std::string& key) {
	auto it = map_.find(key);
	if (it != map_.end()) {
		lru_.erase(it->second.lruPos);
		map_.erase(it);
		return true;
	}
	return false;

}

std::optional<std::string> KVStore::get(const std::string& key) {
	auto it = map_.find(key);
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
