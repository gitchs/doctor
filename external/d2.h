#ifndef EXTERNAL_D2_H
#define EXTERNAL_D2_H
#include <chrono>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include "RuntimeProfile_types.h"
#include "boost/utility/string_view.hpp"
#include "boost/json.hpp"
#include "boost/optional.hpp"
#include "sqlite3.h"

typedef struct AppContext {
  sqlite3* conn;
} AppContext;

typedef struct HDFSStatistics {
  int id;
  boost::string_view format;
  boost::string_view fragment;
  boost::string_view host;
  boost::string_view name;
  int64_t bytes_read = 0;
  int64_t bytes_read_local = 0;
  int64_t bytes_read_short_circuit = 0;
  int64_t decompression_time = 0;
  int64_t num_columns = 0;
  int64_t num_disks_accessed = 0;
  int64_t num_scanners_with_no_reads = 0;
  int64_t peak_memory_usage = 0;
  int64_t per_read_thread_raw_hdfs_throughput = 0;
  int64_t remote_scan_ranges = 0;
  int64_t rows_read = 0;
  int64_t rows_returned = 0;
  int64_t rows_returned_rate = 0;
  int64_t scanner_io_wait_time = 0;
  int64_t total_raw_hdfs_open_file_time = 0;
  int64_t total_raw_hdfs_read_time = 0;
  int64_t total_read_throughput = 0;
  int64_t total_time = 0;
  int64_t scanner_threads_sys_time = 0;
  int64_t scanner_threads_user_time = 0;
  int64_t scanner_threads_wallclock_time = 0;
} HDFSStatistics;

typedef struct Event {
  boost::string_view name;
  std::chrono::nanoseconds checkpoint;
  std::chrono::nanoseconds duration;
} Event;

typedef struct ProfileAnalyzeResult {
  int day;
  std::string query_id;
  boost::string_view start_time;
  boost::string_view end_time;

  boost::json::object summary_counters;
  boost::json::object summary_info_strigns;

  boost::json::object impala_server_counters;
  boost::json::object impala_server_info_strings;

  boost::json::object eprofile_counters;
  boost::json::object eprofile_info_strigns;

  boost::optional<boost::json::object> skew_ops;


  std::map<std::string, std::tuple<int, int, int>> avg_op;
  std::map<std::string, std::vector<std::tuple<int, int, int>>> ops;

} ProfileAnalyzeResult;

#endif  // EXTERNAL_D2_H
