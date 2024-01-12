#ifndef QUERY_PDB_SERVER_DOWNLOADER_H
#define QUERY_PDB_SERVER_DOWNLOADER_H

#include <string>
#include <mutex>
#include <filesystem>
#include <unordered_map>

class downloader {
private:
    static std::string get_relative_path_str(const std::string &name, const std::string &msdl);

public:
    downloader(std::string path, std::string server);

    [[nodiscard]] bool valid() const;

    bool download(const std::string &name, const std::string &msdl);

    std::filesystem::path get_path(const std::string &name, const std::string &msdl);

private:
    bool download_impl(const std::string &relative_path);

    std::pair<std::string, std::string> split_server_name();

private:
    bool valid_;
    std::string path_;
    std::string server_;
    std::pair<std::string, std::string> server_split_;
    std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<std::mutex>> download_mutexs_;
};

#endif //QUERY_PDB_SERVER_DOWNLOADER_H
