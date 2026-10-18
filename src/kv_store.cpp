#include "kv_store.h"

std::size_t KVStore::size() const{
	return map_.size();
}

void KVStore::set(const std::string& key, const std::string& value) {
	map_.insert_or_assign(key, value);
}

bool KVStore::del(const std::string& key) {
	return map_.erase(key) > 0;

}

std::optional<std::string> KVStore::get(const std::string& key) const {
	auto it = map_.find(key);
	if (it != map_.end()) {
		return it->second;
	}
	return std::nullopt;
}