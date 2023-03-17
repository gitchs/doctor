#include <string>
#include <vector>
#include <sstream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include "spdlog/spdlog.h"
#include "sqlite3.h"
#include "d2dbutils.h"
#include "utils.h"


template<unsigned int Size>
std::string GenerateSqlValuesPadding() {
  std::stringstream ss;
  ss << "VALUES(?";
  for (int i=1;i<Size;i++) {
    ss << ", ?";
  }
  ss << ")";
  return ss.str();
}

template<>
std::string GenerateSqlValuesPadding<0>() {
  return "";
}

int InitDatabaseTables(sqlite3* conn) {
  const std::string INIT_STATEMENT_BASIC = "CREATE TABLE IF NOT EXISTS `basic`(\n"
      "`query_id` TEXT NOT NULL UNIQUE\n"
      ",`skew` INT NOT NULL\n"
      ",`skew_ops` TEXT DEFAULT NULL\n"
      ",`summary_counter` TEXT\n"
      ",`summary_info_strings` TEXT\n"
      ",`impala_server_counter` TEXT\n"
      ",`impala_server_info_strings` TEXT\n"
      ",`eprofile_counters` TEXT\n"
      ",`eprofile_info_strings` TEXT\n"
  ")";

  const std::string INIT_STATEMENT_BASIC_IDX_QUERY_ID = "\n"
    "CREATE INDEX IF NOT EXISTS idx_basic_query_id on `basic`(`query_id`)";

  const std::string INIT_STATEMENT_OPS =
      "CREATE TABLE IF NOT EXISTS ops(\n"
      "query_id TEXT\n"
      ",fragment TEXT\n"
      ",host TEXT\n"
      ",name TEXT\n"
      ",counters TEXT\n"
      ")";
  std::vector<const std::string*> sqls = {
    &INIT_STATEMENT_BASIC,
    &INIT_STATEMENT_BASIC_IDX_QUERY_ID,
    &INIT_STATEMENT_OPS,
  };

  char* errmsg = nullptr;
  for (const auto sql : sqls) {
    int rc = sqlite3_exec(conn, sql->c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
      SPDLOG_ERROR("init tables stage failed. Last sql:\n{}", *sql);
      return -1;
    }
  }
  return 0;
}


int InsertQueryBasicInfo(sqlite3* conn, const ProfileAnalyzeResult& result) {
  enum INSERT_COLUMN_INDEX {
    QUERY_ID = 1,
    SKEW,
    SKEW_OPS,
    SUMMARY_COUNTER,
    SUMMARY_INFO_STRINGS,
    IMPALA_SERVER_COUNTER,
    IMPALA_SERVER_INFO_STRINGS,
    EPROFILE_COUNTERS,
    EPROFILE_INFO_STRINGS,
    NUM_COLUMNS_ADD_1, // leave it at tail
  };
  std::string raw_skew_ops;
  std::string raw_summary_counter;
  std::string raw_summary_info_strings;
  std::string raw_impala_server_counter;
  std::string raw_impala_server_info_strings;
  std::string raw_eprofile_counter;
  std::string raw_eprofile_info_strings;

  int rc = 0;
  const static std::string sql = "INSERT INTO `basic`(\n"
      "`query_id`\n"
      ",`skew`\n"
      ",`skew_ops`\n"
      ",`summary_counter`\n"
      ",`summary_info_strings`\n"
      ",`impala_server_counter`\n"
      ",`impala_server_info_strings`\n"
      ",`eprofile_counters`\n"
      ",`eprofile_info_strings`\n"
      ") " + GenerateSqlValuesPadding<NUM_COLUMNS_ADD_1 - 1>();
  ;
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(conn, sql.c_str(), sql.size(), &stmt, nullptr);
  if (rc != SQLITE_OK) {
    SPDLOG_ERROR("sqlite3_prepare_v2 failed.");
    goto CLEANUP;
  }


  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::QUERY_ID, result.query_id.c_str(), result.query_id.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;


  if (result.skew_ops.has_value()) {
    raw_skew_ops = boost::json::serialize(*result.skew_ops);
    rc = sqlite3_bind_int64(stmt, INSERT_COLUMN_INDEX::SKEW, 1);
    assert(rc == SQLITE_OK);
    if (rc != SQLITE_OK) goto CLEANUP;
    rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::SKEW_OPS, raw_skew_ops.c_str(), raw_skew_ops.size(), nullptr);
    assert(rc == SQLITE_OK);
    if (rc != SQLITE_OK) goto CLEANUP;
  } else {
    rc = sqlite3_bind_int64(stmt, INSERT_COLUMN_INDEX::SKEW, 0);
    assert(rc == SQLITE_OK);
    if (rc != SQLITE_OK) goto CLEANUP;
    rc = sqlite3_bind_null(stmt, INSERT_COLUMN_INDEX::SKEW_OPS);
    assert(rc == SQLITE_OK);
    if (rc != SQLITE_OK) goto CLEANUP;
  }



  raw_summary_counter = boost::json::serialize(result.summary_counters);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::SUMMARY_COUNTER, raw_summary_counter.c_str(), raw_summary_counter.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  raw_summary_info_strings = boost::json::serialize(result.summary_info_strigns);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::SUMMARY_INFO_STRINGS, raw_summary_info_strings.c_str(), raw_summary_info_strings.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  raw_impala_server_counter = boost::json::serialize(result.impala_server_counters);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::IMPALA_SERVER_COUNTER, raw_impala_server_counter.c_str(), raw_impala_server_counter.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  raw_impala_server_info_strings = boost::json::serialize(result.impala_server_info_strings);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::IMPALA_SERVER_INFO_STRINGS, raw_impala_server_info_strings.c_str(), raw_impala_server_info_strings.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  raw_eprofile_counter = boost::json::serialize(result.eprofile_counters);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::EPROFILE_COUNTERS, raw_eprofile_counter.c_str(), raw_eprofile_counter.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  raw_eprofile_info_strings = boost::json::serialize(result.eprofile_info_strigns);
  rc = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::EPROFILE_INFO_STRINGS, raw_eprofile_info_strings.c_str(), raw_eprofile_info_strings.size(), nullptr);
  assert(rc == SQLITE_OK);
  if (rc != SQLITE_OK) goto CLEANUP;

  do {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
      break;
    } else if (rc == SQLITE_CONSTRAINT) {
      const char* errmsg = sqlite3_errmsg(conn);
      SPDLOG_ERROR("handle query [{}] failed, error: {}", result.query_id, errmsg);
      break;
    } else if (rc == SQLITE_ERROR) {
      SPDLOG_ERROR("Insert record failed. RC = {}", rc);
      break;
    } else if (rc == SQLITE_BUSY) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
      continue;
    } else {
      const char* errmsg = sqlite3_errmsg(conn);
      SPDLOG_ERROR("handle query [{}] failed, rc = {}, error: {}", result.query_id, rc, errmsg);
      break;
    }
  } while(true);
CLEANUP:
  if (stmt != nullptr)
    sqlite3_finalize(stmt);
  return rc;
}



int InsertQuerySkewOperatorInfo(sqlite3* conn, const ProfileAnalyzeResult& result, const std::shared_ptr<impala::TRuntimeProfileTree>& tree) {
  enum INSERT_COLUMN_INDEX {
    QUERY_ID = 1,
    FRAGMENT,
    HOST,
    NAME,
    COUNTERS,
    NUM_COLUMNS_ADD_1, // leave it at tail
  };
  if (!result.skew_ops.has_value())
    return SQLITE_OK;

  int retval = 0;
  const static std::string sql = "INSERT INTO ops(\n"
"query_id\n"
",fragment\n"
",host\n"
",name\n"
",counters\n"
")" + GenerateSqlValuesPadding<INSERT_COLUMN_INDEX::NUM_COLUMNS_ADD_1 - 1>();
  sqlite3_stmt* stmt = nullptr;
  retval = sqlite3_prepare_v2(conn, sql.c_str(), sql.size(), &stmt, nullptr);
  if (retval != SQLITE_OK)
    goto CLEANUP;
  for (const auto& it : *result.skew_ops) {
    const auto& name = it.key();
    const auto& ops = result.ops.find(name);
    if (ops == result.ops.end())
      continue;
    for (const auto& op : ops->second) {
      const auto& node = tree->nodes.at(std::get<0>(op));
      const auto& fragment = tree->nodes.at(std::get<1>(op));
      const auto& host = tree->nodes.at(std::get<2>(op));
      boost::json::object counters;
      for (const auto& counter : node.counters) {
        counters[counter.name] = counter.value;
      }
      std::string raw_counters = boost::json::serialize(counters);
      retval = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::QUERY_ID, result.query_id.c_str(), result.query_id.size(), nullptr);
      if (retval != SQLITE_OK)
        goto CLEANUP;
      retval = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::FRAGMENT, fragment.name.c_str(), fragment.name.size(), nullptr);
      if (retval != SQLITE_OK)
        goto CLEANUP;
      retval = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::HOST, host.name.c_str(), host.name.size(), nullptr);
      if (retval != SQLITE_OK)
        goto CLEANUP;
      retval = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::NAME, node.name.c_str(), node.name.size(), nullptr);
      if (retval != SQLITE_OK)
        goto CLEANUP;
      retval = sqlite3_bind_text(stmt, INSERT_COLUMN_INDEX::COUNTERS, raw_counters.c_str(), raw_counters.size(), nullptr);
      if (retval != SQLITE_OK)
        goto CLEANUP;
      do {
        retval = sqlite3_step(stmt);
        if (retval == SQLITE_DONE) {
          break;
        } else if (retval == SQLITE_CONSTRAINT) {
          const char* errmsg = sqlite3_errmsg(conn);
          SPDLOG_ERROR("handle query [{}] failed, error: {}", result.query_id, errmsg);
          goto CLEANUP;
        } else if (retval == SQLITE_ERROR) {
          SPDLOG_ERROR("Insert record failed. RC = {}", retval);
          goto CLEANUP;
        } else if (retval == SQLITE_BUSY) {
          boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
          continue;
        } else {
          const char* errmsg = sqlite3_errmsg(conn);
          SPDLOG_ERROR("handle query [{}] failed, retval = {}, error: {}", result.query_id, retval, errmsg);
          goto CLEANUP;
        }
      } while(true);
      sqlite3_reset(stmt);
    }
  }
CLEANUP:
  if (stmt != nullptr) {
    if (retval != SQLITE_OK && retval != SQLITE_DONE) {
      const char* errmsg = sqlite3_errmsg(conn);
      SPDLOG_ERROR("handle query [{}] failed, rc = {}, error: {}", result.query_id, retval, errmsg);
    }
    sqlite3_finalize(stmt);
  }
  return retval;
}
