#include "d2utils.h"
#include <boost/optional.hpp>
#include <string>
#include "RuntimeProfile_types.h"
#include "boost/utility/string_view.hpp"
#include "re2/re2.h"
#include "spdlog/spdlog.h"
#include "sqlite3.h"
#include "utils.h"

int ExtractOperatorId(const std::string& name) {
  re2::StringPiece sp(name);
  re2::RE2 re("\\(id=(\\d+)\\)");
  std::string val = "";
  if (!re2::RE2::FindAndConsume(&sp, re, &val))
    return -1;
  return std::stoi(val);
}

int ProcessSummaryNode(ProfileAnalyzeResult& result,
                       std::shared_ptr<impala::TRuntimeProfileTree>& tree) {
  int retval = SUMMARY_NODE_INDEX;
  auto& summary_node = tree->nodes.at(SUMMARY_NODE_INDEX);
  assert(summary_node.name == "Summary");

  for (const auto& info : summary_node.info_strings) {
    result.summary_info_strigns[info.first] = info.second;
  }

  for (const auto& counter : summary_node.counters) {
    result.summary_counters[counter.name] = counter.value;
  }

  for (int offset = summary_node.num_children; offset; offset--) {
    ++retval;
    assert(retval < tree->nodes.size());
    const auto& node = tree->nodes.at(retval);
    offset += node.num_children;
  }
  return ++retval;
}

int ProcessExecutionProfileNode(
    ProfileAnalyzeResult& result,
    std::shared_ptr<impala::TRuntimeProfileTree>& tree,
    int idx) {
  if( idx >= tree->nodes.size())
    return idx;
  int retval = idx;
  const auto& eprofile = tree->nodes.at(idx);
  const static std::string eprofile_name_prefix = "Execution Profile ";
  if (!StartsWith(eprofile.name.begin(), eprofile.name.end(),
                  eprofile_name_prefix.begin(), eprofile_name_prefix.end()))
    return -1;
  for (const auto& counter : eprofile.counters) {
    result.eprofile_counters[counter.name] = counter.value;
  }
  for (const auto& it : eprofile.info_strings) {
    result.eprofile_info_strigns[it.first] = boost::string_view(it.second);
  }
  return retval + 1;
}

int ProcessImpalaServerNode(ProfileAnalyzeResult& result,
                            std::shared_ptr<impala::TRuntimeProfileTree>& tree,
                            int idx) {
  if (idx >= tree->nodes.size())
    return idx;
  const auto& node = tree->nodes.at(idx);
  assert(node.name == "ImpalaServer");
  for (const auto& it : node.info_strings) {
    result.impala_server_info_strings[it.first] = boost::string_view(it.second);
  }
  for (const auto& counter : node.counters) {
    result.impala_server_counters[counter.name] = counter.value;
  }
  return idx + 1;
}

int IterateExecutionProfileOperatorNodes(
    ProfileAnalyzeResult& result,
    std::shared_ptr<impala::TRuntimeProfileTree>& tree,
    int idx) {
  if (idx >= tree->nodes.size())
    return idx;
  int idx_fragment = -1;
  int idx_host = -1;
  boost::optional<const impala::TRuntimeProfileNode&> fragment_node;
  boost::optional<const impala::TRuntimeProfileNode&> host_node;

  for (; idx < tree->nodes.size(); idx++) {
    const auto& node = tree->nodes.at(idx);

    if (StartsWith(node.name.begin(), node.name.end(),
                   AVERAGE_FRAGMENT_NAME_PREFIX.begin(),
                   AVERAGE_FRAGMENT_NAME_PREFIX.end())) {
      idx_fragment = idx;
      idx_host = -1;
      fragment_node.emplace(node);
      host_node.reset();
    } else if (StartsWith(node.name.begin(), node.name.end(),
                          FRAGMENT_NAME_PREFIX0.begin(),
                          FRAGMENT_NAME_PREFIX0.end()) ||
               StartsWith(node.name.begin(), node.name.end(),
                          FRAGMENT_NAME_PREFIX1.begin(),
                          FRAGMENT_NAME_PREFIX1.end())) {
      idx_fragment = idx;
      idx_host = -1;
      fragment_node.emplace(node);
      host_node.reset();
      continue;
    } else if (StartsWith(node.name.begin(), node.name.end(),
                          INSTANCE_NAME_PREFIX.begin(),
                          INSTANCE_NAME_PREFIX.end())) {
      idx_host = idx;
      host_node.emplace(node);
      continue;
    }

    if (!fragment_node.has_value())
      continue;

    int op_id = ExtractOperatorId(node.name);
    if (op_id == -1)
      continue;  // not operator

    if (StartsWith(fragment_node->name.begin(), fragment_node->name.end(),
                   AVERAGE_FRAGMENT_NAME_PREFIX.begin(),
                   AVERAGE_FRAGMENT_NAME_PREFIX.end())) {
      result.avg_op.insert(std::make_pair(
          node.name, std::make_tuple<>(idx, idx_fragment, idx_host)));
      continue;
    }
    result.ops[node.name].push_back(
        std::make_tuple<>(idx, idx_fragment, idx_host));
  }
  return idx;
}


boost::json::array HDFSOperators2JSON(const ProfileAnalyzeResult& result, const std::shared_ptr<impala::TRuntimeProfileTree>& tree) {
  boost::json::array retval;
  for (const auto& it: result.ops) {
    const std::string& name = it.first;
    if (!StartsWith(name.begin(), name.end(), HDFS_SCAN_NAME_PREFIX.begin(), HDFS_SCAN_NAME_PREFIX.end()))
      continue;
    for (const auto& op : it.second) {
      const auto& node = tree->nodes.at(std::get<0>(op));
      const auto& fragment = tree->nodes.at(std::get<1>(op));
      const auto& host = tree->nodes.at(std::get<2>(op));
      boost::json::object item;
      item["fragment"] = fragment.name;
      item["host"] = host.name;
      for (const auto& counter : node.counters) {
        item[counter.name] = counter.value;
      }
      retval.push_back(item);
    }
  }
  return retval;
}
