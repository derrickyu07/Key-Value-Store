#include <iostream>
#include <string>
#include "command.h"
#include "kv_store.h"

int main() {
	KVStore store;
	std::string line;

	while (true) {
		std::cout << "> ";
		if (!std::getline(std::cin, line)) break;

		Command cmd = parse(line);
		switch (cmd.type) {
		case CommandType::Set:
			store.set(cmd.key, cmd.value);
			std::cout << "OK\n";
			break;

		case CommandType::Get: {
			auto value = store.get(cmd.key);
			std::cout << (value ? *value : "(nil)") << '\n';
			break;
		}
		case CommandType::Del:
			std::cout << (store.del(cmd.key) ? "OK" : "(nil)") << '\n';
			break;

		case CommandType::Exit:
			return 0;

		case CommandType::Invalid:
			std::cout << "ERR invalid command\n";
			break;
		}
	}
	return 0;
}