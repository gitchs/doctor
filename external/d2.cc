#include <boost/optional.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include "boost/asio.hpp"
#include "boost/bind/bind.hpp"
#include "boost/chrono.hpp"
#include "boost/json.hpp"
#include "boost/thread/thread.hpp"
#include "boost/utility/string_view.hpp"

#include "avro/DataFile.hh"
#include "avro/Decoder.hh"
#include "avro/Generic.hh"
#include "avro/GenericDatum.hh"
#include "avro/Types.hh"

#include "re2/re2.h"
#include "spdlog/spdlog.h"
#include "sqlite3.h"

#include "d2.h"
#include "d2dbutils.h"
#include "d2strategies.h"
#include "d2utils.h"
#include "utils.h"

static boost::mutex m;
static int QUEUE_SIZE = 0;
const static int MAX_QUEUE_SIZE = 4096;

static AppContext app_context;

void PoolTaskDelegation(const std::string& profile) {
  ProfileAnalyzeResult result;
  std::shared_ptr<impala::TRuntimeProfileTree> tree = DecodeProfile(profile);
  if (tree->nodes.size() <= 0) {
    return;
  }
  auto& query_node = tree->nodes.at(QUERY_NODE_INDEX);
  assert(StartsWith(query_node.name.begin(), query_node.name.end(),
                    NODE_QUERY_PREFIX.begin(), NODE_QUERY_PREFIX.end()));
  result.query_id = query_node.name.substr(10, 33);
  SPDLOG_INFO("working on query \"{}\"", result.query_id);

  int idx = ProcessSummaryNode(result, tree);
  if (idx == -1)
    return;
  idx = ProcessImpalaServerNode(result, tree, idx);
  if (idx == -1)
    return;
  idx = ProcessExecutionProfileNode(result, tree, idx);

  idx = IterateExecutionProfileOperatorNodes(result, tree, idx);
  assert(idx == tree->nodes.size());
  boost::json::object skew_ops = strategies::AnalyzeOperatorTimeRange(result, tree);
  if (skew_ops.size() > 0)
    result.skew_ops.emplace(std::move(skew_ops));
  int rc = InsertQueryBasicInfo(app_context.conn, result);
  SPDLOG_INFO("InsertQueryBasicInfo rc = {}", rc);
  if (result.skew_ops.has_value()) {
    rc = InsertQuerySkewOperatorInfo(app_context.conn, result, tree);
    SPDLOG_INFO("InsertQuerySkewOperatorInfo rc = {}", rc);
  }
  return;
}

void PoolTask(const std::string profile) {
  PoolTaskDelegation(profile);
  m.lock();
  QUEUE_SIZE -= 1;
  // std::cerr << "QUEUE_SIZE = " << QUEUE_SIZE << "\n";
  m.unlock();
}

int main(int argc, char** args) {
  if (argc < 2) {
    SPDLOG_INFO("no input avro_file");
    return 1;
  }

#ifdef ENABLE_THREAD_POOL
  SPDLOG_INFO("run in multi thread mode");
#else
  SPDLOG_INFO("run in single thread mode");
#endif  // ENABLE_THREAD_POOL

  std::shared_ptr<avro::DataFileReader<avro::GenericDatum>> reader =
      std::make_shared<avro::DataFileReader<avro::GenericDatum>>(args[1]);
  avro::GenericDatum* datum = new avro::GenericDatum(reader->dataSchema());
  const auto& schema = reader->dataSchema();
  const auto& schema_root = schema.root();
  size_t schema_name_len = schema_root->names();
  size_t profile_index = -1;
  for (size_t i = 0; i < schema_name_len; i++) {
    std::string name = schema_root->nameAt(i);
    if (name == "profile") {
      profile_index = i;
      break;
    }
  }
  assert(profile_index != -1);

  sqlite3_open("./meta-d2.db", &app_context.conn);
  InitDatabaseTables(app_context.conn);

#ifdef ENABLE_THREAD_POOL
  boost::asio::thread_pool pool(4);
#endif  // ENABLE_THREAD_POOL

  bool has_more = false;
  do {
    has_more = reader->read(*datum);
    if (!has_more) {
      break;
    }
    avro::GenericRecord record = datum->value<avro::GenericRecord>();
    const auto& field = record.fieldAt(profile_index);
    const auto ftype = field.type();
    if (ftype == avro::AVRO_NULL) {
      continue;
    }
    if (ftype == avro::AVRO_STRING) {
      const std::string& profile = field.value<std::string>();
#ifdef ENABLE_THREAD_POOL
      do {
        m.lock();
        if (QUEUE_SIZE >= MAX_QUEUE_SIZE) {
          m.unlock();
          std::cerr << "QUEUE FULL, WAIT NEXT LOOP"
                    << "\n";
          boost::this_thread::sleep_for(boost::chrono::seconds(2));
          continue;
        }
        boost::asio::post(pool, boost::bind(PoolTask, std::move(profile)));
        QUEUE_SIZE++;
        m.unlock();
        break;
      } while (true);
#else
      PoolTask(std::move(profile));
#endif        // ENABLE_THREAD_POOL
      // break;  // TODO: remove this line when development is done.
    }
  } while (has_more);

#ifdef ENABLE_THREAD_POOL
  pool.join();
#endif  // ENABLE_THREAD_POOL
  sqlite3_close(app_context.conn);
  return 0;
}
