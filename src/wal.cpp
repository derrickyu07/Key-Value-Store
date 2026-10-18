#include "wal.h"
#include "command.h"
WriteAheadLog::WriteAheadLog(const std::string& path)
    : path_(path), out_(path, std::ios::app) {
}

void WriteAheadLog::appendSet(const std::string& key, const std::string& value) {

    out_ << "SET " << key << ' ' << value << '\n';
    out_.flush();
}
void WriteAheadLog::appendDel(const std::string& key) {
    out_ << "DEL " << key << '\n';
    out_.flush();
}
void WriteAheadLog::replay(KVStore& store) const {
    std::ifstream in(path_);
    std::string line;
    while (std::getline(in, line)) {
        Command cmd = parse(line);
        switch (cmd.type) {
        case CommandType::Set:
            store.set(cmd.key, cmd.value);
            break;
        case CommandType::Del:
            store.del(cmd.key);
            break;
        default:
            break;
        }
    }
}