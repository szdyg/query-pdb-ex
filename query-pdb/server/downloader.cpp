#include "downloader.h"

#include <httplib.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <regex>
#include <sstream>

downloader::downloader(std::string path, std::string server) {
  valid_ = false;
  path_ = path;
  server_ = server;

  spdlog::info("create downloader, path: {}, server: {}", path_, server_);
  if (server_.empty() || path_.empty()) {
    spdlog::error("invalid downloader, path: {}, server: {}", path_, server_);
    return;
  }

  server_split_ = split_server_name();
  if (server_split_.first.empty()) {
    spdlog::error("split server name failed, server: {}", server_);
    return;
  }

  valid_ = true;
}

bool downloader::valid() const { return valid_; }

bool downloader::download(const std::string &name, const std::string &msdl) {
  std::string relative_path = get_relative_path_str(name, msdl);
  auto path = get_path(name, msdl);
  spdlog::info("lookup pdb, path: {}", relative_path);

  if (std::filesystem::exists(path)) {
    spdlog::info("pdb already exists, path: {}", relative_path);
    return true;
  }

  return download_impl(relative_path);
}

std::string downloader::get_relative_path_str(const std::string &name, const std::string &msdl) {
  std::stringstream ss;
  ss << std::hex << std::uppercase;
  ss << name << '/' << msdl << '/' << name;
  return ss.str();
}

std::filesystem::path downloader::get_path(const std::string &name, const std::string &msdl) {
  std::string relative_path = get_relative_path_str(name, msdl);
  auto path = std::filesystem::path(path_).append(relative_path);
  return path;
}

bool downloader::download_impl(const std::string &relative_path) {
  spdlog::info("download pdb, path: {}", relative_path);

  std::lock_guard<std::mutex> lock(mutex_);
  auto path = std::filesystem::path(path_).append(relative_path);
  std::filesystem::create_directories(path.parent_path());

  httplib::Client client(server_split_.first);
  client.set_follow_location(true);
  auto res = client.Get(server_split_.second + relative_path);
  if (res.error() != httplib::Error::Success || res->status != 200) {
    spdlog::error("failed to download pdb, path: {}", relative_path);
    return false;
  }

  auto tmp_path = path;
  tmp_path.replace_extension(".tmp");
  std::ofstream f(tmp_path, std::ios::binary);
  if (!f.is_open()) {
    spdlog::error("failed to open file, path: {}", tmp_path.string());
    return false;
  }
  f.write(res->body.c_str(), static_cast<std::streamsize>(res->body.size()));
  f.close();

  std::filesystem::rename(tmp_path, path);
  spdlog::info("download pdb success, path: {}", relative_path);
  return true;
}

std::pair<std::string, std::string> downloader::split_server_name() {
  std::regex regex(R"(^((?:(?:http|https):\/\/)?[^\/]+)(\/.*)$)");
  std::smatch match;

  if (!std::regex_match(server_, match, regex)) {
    return {};
  }

  return {match[1].str(), match[2].str()};
}
