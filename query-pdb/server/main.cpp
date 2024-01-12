#include <httplib.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/spdlog.h>

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <set>
#include <utility>

#include "downloader.h"
#include "pdb_parser.h"

int main(int argc, char *argv[]) {
  cxxopts::Options option_parser("query-pdb", "pdb query server");
  option_parser.add_options()
      ("ip", "ip address", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "port", cxxopts::value<uint16_t>()->default_value("8080"))
      ("path", "download path", cxxopts::value<std::string>()->default_value("pdb"))
      ("server", "download server",cxxopts::value<std::string>()->default_value("http://msdl.szdyg.cn/download/symbols/"))
      ("log", "write log to file", cxxopts::value<bool>()->default_value("false"))("h,help", "print help");

  auto parse_result = option_parser.parse(argc, argv);

  if (parse_result.count("help")) {
    std::cout << option_parser.help() << std::endl;
    return 0;
  }

  const auto ip = parse_result["ip"].as<std::string>();
  const auto port = parse_result["port"].as<uint16_t>();
  const auto download_path = parse_result["path"].as<std::string>();
  const auto download_server = parse_result["server"].as<std::string>();
  const auto log_to_file = parse_result["log"].as<bool>();

  if (log_to_file) {
    spdlog::set_default_logger(spdlog::daily_logger_mt("query-pdb", "server.log"));
  }

  downloader storage(download_path, download_server);
  if (!storage.valid()) {
    spdlog::error("exit due to downloader invalid");
    return 1;
  }

  httplib::Server server;
  server.set_exception_handler([](const auto &req, auto &res, std::exception_ptr ep) {
    std::string content;
    try {
      std::rethrow_exception(ep);
    } catch (std::exception &e) {
      content = e.what();
    } catch (...) {
      content = "Unknown Exception";
    }
    res.set_content(content, "plain/text");
    res.status = 500;

    spdlog::error("exception: {}", content);
  });

  // example:
  // {
  //     "name": "ntdll.pdb",
  //     "msdl": "ABCDEF...1",
  //     "query": [
  //         "Name1",
  //         "Name2",
  //         ...
  //     ]
  // }

  server.Post("/symbol", [&storage](const httplib::Request &req, httplib::Response &res) {
    spdlog::info("symbol request: {}", req.body);
    auto body = nlohmann::json::parse(req.body);
    auto name = body["name"].get<std::string>();
    auto msdl = body["msdl"].get<std::string>();
    auto query = body["query"].get<std::set<std::string>>();

    // download pdb
    if (!storage.download(name, msdl)) {
      throw std::runtime_error("download failed");
    }

    // parse pdb
    pdb_parser parser(storage.get_path(name, msdl).string());
    auto result = parser.get_symbols(query);
    res.set_content(result.dump(), "application/json");
  });

  // example:
  // {
  //     "name": "ntdll.pdb",
  //     "msdl": "ABCDEF...1",
  //     "query": {
  //         "struct1",
  //         "struct2",
  //         ...
  //     }
  // }

  server.Post("/struct", [&storage](const httplib::Request &req, httplib::Response &res) {
    spdlog::info("struct request: {}", req.body);
    auto body = nlohmann::json::parse(req.body);
    auto name = body["name"].get<std::string>();
    auto msdl = body["msdl"].get<std::string>();
    auto query = body["query"].get<std::set<std::string>>();

    // download pdb
    if (!storage.download(name, msdl)) {
      throw std::runtime_error("download failed");
    }

    // parse pdb
    pdb_parser parser(storage.get_path(name, msdl).string());
    auto result = parser.get_struct(query);
    res.set_content(result.dump(), "application/json");
  });

  // example:
  // {
  //     "name": "ntdll.pdb",
  //     "guid": "ABCDEF...",
  //     "age": 1
  //     "query": {
  //         "enum1",
  //         "enum2",
  //         ...
  //     }
  // }

  server.Post("/enum", [&storage](const httplib::Request &req, httplib::Response &res) {
    spdlog::info("enum request: {}", req.body);
    auto body = nlohmann::json::parse(req.body);
    auto name = body["name"].get<std::string>();
    auto msdl = body["msdl"].get<std::string>();
    auto query = body["query"].get<std::set<std::string>>();

    // download pdb
    if (!storage.download(name, msdl)) {
      throw std::runtime_error("download failed");
    }

    // parse pdb
    pdb_parser parser(storage.get_path(name, msdl).string());
    auto result = parser.get_enum(query);
    res.set_content(result.dump(), "application/json");
  });

  server.listen(ip, port);
  return 0;
}
