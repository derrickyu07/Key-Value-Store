#include "database.h"
#include "wal.h"

Database::Database(const std::string& logPath, std::size_t capacity):store_(capacity), log_(logPath) {
	log_.replay(store_);
}

std::string Database::execute(const Command& cmd) {
	switch (cmd.type) {
	case CommandType::Set:
		log_.appendSet(cmd.key, cmd.value);
		store_.set(cmd.key, cmd.value);
		return "OK";

	case CommandType::Get: {
		auto value = store_.get(cmd.key);
		return (value ? *value : "(nil)");
	}
	case CommandType::Del:
		log_.appendDel(cmd.key);
		return (store_.del(cmd.key) ? "OK" : "(nil)");

	case CommandType::Exit:
		return "";

	case CommandType::Invalid:
		return "ERR invalid command";
	}
	return "ERR invalid command";
}
