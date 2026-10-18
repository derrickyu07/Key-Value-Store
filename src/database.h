class Database {
public:
    Database(const std::string& logPath, std::size_t capacity);
    std::string execute(const Command& cmd);   // returns the text to print

private:
    KVStore store_;
    WriteAheadLog log_;
};