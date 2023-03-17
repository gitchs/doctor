#include <iostream>
#include <chrono>
#include <vector>
#include "boost/json.hpp"
#include "d2.h"
#include "d2utils.h"
#include "spdlog/spdlog.h"
#include "utils.h"

namespace strategies {


const static int64_t IGNORE_DURATION_WINDOW = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(30)).count();
const static int64_t IGNORE_OP_MAX_DURATION = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(15)).count();
const static int64_t DEFAULT_OPERATOR_MAX_ACCEPTABLE_RANGE = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(25)).count();
const static int64_t UNIGNORE_MIN_DURATION = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(30)).count();

boost::json::object AnalyzeOperatorTimeRange(
    const ProfileAnalyzeResult& result,
    const std::shared_ptr<impala::TRuntimeProfileTree>& tree) {
  boost::json::object retval;


  const auto& it = result.summary_info_strigns.find("Duration(ms)");
  if (it == result.summary_info_strigns.end())
    return retval;

  const auto& duration_str = it->value().as_string();
  int64_t duration_ms = std::stoll(duration_str.c_str());
  if (duration_ms < IGNORE_DURATION_WINDOW)
    return retval;

  for (const auto& it : result.ops) {
    int64_t min_ts = 0;
    int64_t max_ts = 0;
    boost::optional<const impala::TRuntimeProfileNode&> slowest_node;
    boost::optional<const impala::TRuntimeProfileNode&> fastest_node;

    const auto& op = it.second.front();
    const auto& node = tree->nodes.at(std::get<0>(op));
    const auto& fragment = tree->nodes.at(std::get<1>(op));
    for (int i = 0; i < node.counters.size(); i++) {
      const auto& counter = node.counters.at(i);
      if (counter.name == "TotalTime") {
        min_ts = counter.value;
        max_ts = counter.value;
        slowest_node.emplace(tree->nodes.at(std::get<2>(op)));
        fastest_node.emplace(tree->nodes.at(std::get<2>(op)));
        break;
      }
    }
    if (!slowest_node.has_value())
      continue;
    for (int i = 1; i < it.second.size(); i++) {
      const auto& op = it.second.at(i);
      const auto& node = tree->nodes.at(std::get<0>(op));

      for (const auto& counter : node.counters) {
        if (counter.name == "TotalTime") {
          if (min_ts > counter.value) {
            min_ts = counter.value;
            fastest_node.emplace(tree->nodes.at(std::get<2>(op)));
          }
          if (max_ts < counter.value) {
            max_ts = counter.value;
            slowest_node.emplace(tree->nodes.at(std::get<2>(op)));
          }
          break;
        }
      }
    }

    if (max_ts == 0)
      continue;
    if (max_ts < IGNORE_OP_MAX_DURATION)
      continue;


    int64_t ts_range = max_ts - min_ts;
    if (ts_range < DEFAULT_OPERATOR_MAX_ACCEPTABLE_RANGE)
      continue;


    boost::json::object op_stat;
    op_stat["skew"] = true;
    op_stat["fragment"] = fragment.name;
    op_stat["ts_range_ms"] = ns2ms(ts_range);
    op_stat["slowest_node"] =
        slowest_node.has_value() ? slowest_node->name : "[UNKNOWN]";
    op_stat["fastest_node"] =
        fastest_node.has_value() ? fastest_node->name : "[UNKNOWN]";
    op_stat["min_ts_ms"] = ns2ms(min_ts);
    op_stat["max_ts_ms"] = ns2ms(max_ts);
    retval[node.name] = op_stat;
  }
  return retval;
}
};  // namespace strategies
