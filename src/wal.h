class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& path);

    void appendSet(const std::string& key, const std::string& value);
    void appendDel(const std::string& key);
    void replay(KVStore& store);   // read file, apply each line to store

private:
    std::string path_;
    std::ofstream out_;   // opened in append mode
};