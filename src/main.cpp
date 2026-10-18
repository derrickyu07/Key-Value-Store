#include <iostream>
#include <string>
#include "command.h"
#include "kv_store.h"
#include "database.h"

int main() {
    Database db("kvstore.log");
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        Command cmd = parse(line);
        if (cmd.type == CommandType::Exit) return 0;
        std::cout << db.execute(cmd) << '\n';
    }
}