#ifndef QUERY_PDB_SERVER_PDB_PARSER_H
#define QUERY_PDB_SERVER_PDB_PARSER_H

#include <PDB.h>
#include <PDB_DBIStream.h>
#include <PDB_InfoStream.h>
#include <PDB_RawFile.h>
#include <PDB_TPIStream.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <stdexcept>
#include "handle_guard.h"
#include <nlohmann/json.hpp>

class pdb_parser {
 public:
  explicit pdb_parser(const std::string &filename);

  nlohmann::json get_symbols(const std::set<std::string> &names) const;

  nlohmann::json get_struct(const std::set<std::string> &names) const;

  nlohmann::json get_enum(const std::set<std::string> &names) const;

 private:
  handle_guard file_{};

  static nlohmann::json get_symbols_impl(
      const PDB::RawFile &raw_file,
      const PDB::DBIStream &dbi_stream,
      const PDB::TPIStream &tpi_stream,
      const std::set<std::string> &names);

  static nlohmann::json get_struct_impl(
      const PDB::RawFile &raw_file,
      const PDB::DBIStream &dbi_stream,
      const PDB::TPIStream &tpi_stream,
      const std::set<std::string> &names);

  static std::map<std::string, int64_t > get_struct_single(
      const PDB::TPIStream &tpi_stream,
      const PDB::CodeView::TPI::Record *record);

  static nlohmann::json get_enum_impl(
      const PDB::RawFile &raw_file,
      const PDB::DBIStream &dbi_stream,
      const PDB::TPIStream &tpi_stream,
      const std::set<std::string> &names);

  static std::map<std::string, int64_t> get_enum_single(
      const PDB::CodeView::TPI::Record *record,
      uint8_t underlying_type_size);

  template <typename F, typename... Args>
  auto call_with_pdb_stream(F f, Args &&...args) const {
    // sanity check
    if (!file_.get().baseAddress ||
        PDB::ValidateFile(file_.get().baseAddress) != PDB::ErrorCode::Success) {
      throw std::runtime_error("invalid PDB file");
    }

    const PDB::RawFile raw_file = PDB::CreateRawFile(file_.get().baseAddress);
    if (PDB::HasValidDBIStream(raw_file) != PDB::ErrorCode::Success) {
      throw std::runtime_error("invalid DBI stream");
    }

    const PDB::InfoStream info_stream(raw_file);
    if (info_stream.UsesDebugFastLink()) {
      throw std::runtime_error("invalid info stream");
    }

    const PDB::DBIStream dbi_stream = PDB::CreateDBIStream(raw_file);
    if (dbi_stream.HasValidImageSectionStream(raw_file) != PDB::ErrorCode::Success ||
        dbi_stream.HasValidPublicSymbolStream(raw_file) != PDB::ErrorCode::Success ||
        dbi_stream.HasValidGlobalSymbolStream(raw_file) != PDB::ErrorCode::Success ||
        dbi_stream.HasValidSectionContributionStream(raw_file) != PDB::ErrorCode::Success) {
      throw std::runtime_error("invalid DBI streams");
    }

    const PDB::TPIStream tpi_stream = PDB::CreateTPIStream(raw_file);
    if (PDB::HasValidTPIStream(raw_file) != PDB::ErrorCode::Success) {
      throw std::runtime_error("invalid TPI stream");
    }

    return f(raw_file, dbi_stream, tpi_stream, std::forward<Args>(args)...);
  }
};

#endif  // QUERY_PDB_SERVER_PDB_PARSER_H
