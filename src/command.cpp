#include "command.h"
#include <sstream>

Command parse(const std::string& line) {
	std::istringstream iss(line);

	std::string name;
	if (!(iss >> name)) return Command{};


	Command cmd;
	if (name == "EXIT") {
		cmd.type = CommandType::Exit;
		return cmd;
	}

	if (!(iss >> cmd.key)) return Command{};

	if (name == "SET") {
		std::getline(iss, cmd.value);
		auto start = cmd.value.find_first_not_of(' ');
        if (start == std::string::npos) return Command{};   // no value given
		cmd.value = cmd.value.substr(start);
		cmd.type = CommandType::Set;
	}
	else if (name == "GET") {
		cmd.type = CommandType::Get;
	}
	else if (name == "DEL") {
		cmd.type = CommandType::Del;
	}
	else {
		return Command{};
	}

	return cmd;
}