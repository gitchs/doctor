#ifndef EXTERNAL_D2_UTILS_H
#define EXTERNAL_D2_UTILS_H
#include <boost/optional.hpp>
#include <memory>
#include "RuntimeProfile_types.h"
#include "boost/json.hpp"
#include "boost/utility/string_view.hpp"
#include "d2.h"
#include "sqlite3.h"

const static int QUERY_NODE_INDEX = 0;
const static int SUMMARY_NODE_INDEX = 1;

const static std::string NODE_QUERY_PREFIX = "Query ";
const static std::string NODE_SUMMARY_NAME = "Summary";
const static std::string QUERY_TYPE_QUERY = "QUERY";
const static std::string QUERY_TYPE_DML = "DML";

const static std::string AVERAGE_FRAGMENT_NAME_PREFIX = "Averaged Fragment F";
const static std::string FRAGMENT_NAME_PREFIX0 = "Fragment F";
const static std::string FRAGMENT_NAME_PREFIX1 = "Coordinator Fragment F";
const static std::string INSTANCE_NAME_PREFIX = "Instance ";
const static std::string HDFS_SCAN_NAME_PREFIX = "HDFS_SCAN_NODE (";



boost::json::array HDFSOperators2JSON(const ProfileAnalyzeResult& reuslt, const std::shared_ptr<impala::TRuntimeProfileTree>& tree);

int ExtractOperatorId(const std::string& name);

int ProcessSummaryNode(ProfileAnalyzeResult& result,
                       std::shared_ptr<impala::TRuntimeProfileTree>& tree);

int ProcessExecutionProfileNode(
    ProfileAnalyzeResult& result,
    std::shared_ptr<impala::TRuntimeProfileTree>& tree,
    int idx);

int IterateExecutionProfileOperatorNodes(
    ProfileAnalyzeResult& result,
    std::shared_ptr<impala::TRuntimeProfileTree>& tree,
    int idx);
int ProcessImpalaServerNode(ProfileAnalyzeResult& result,
                            std::shared_ptr<impala::TRuntimeProfileTree>& tree,
                            int idx);

inline int64_t SafeNodeInfo2Int64(const impala::TRuntimeProfileNode& node,
                                  const std::string& key) {
  if (node.info_strings.find(key) == node.info_strings.end())
    return -1;
  const std::string& val = node.info_strings.at(key);
  for (const auto& it : val) {
    if (__builtin_expect(it < '0' || it > '9', 0))
      return -1;
  }
  return std::stoll(val);
}

inline boost::string_view SafeNodeInfoString(
    const impala::TRuntimeProfileNode& node,
    const std::string& key) {
  const static std::string empty_string = "";
  const auto& info = node.info_strings.find(key);
  if (info == node.info_strings.end())
    return boost::string_view(empty_string);
  return boost::string_view(info->second);
}

#endif  // EXTERNAL_D2_UTILS_H
