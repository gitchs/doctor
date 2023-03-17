#ifndef EXTERNAL_D2_STRATEGIES_H
#define EXTERNAL_D2_STRATEGIES_H
#include <memory>
#include "boost/json.hpp"
#include "d2.h"

namespace strategies {

boost::json::object AnalyzeOperatorTimeRange(
    const ProfileAnalyzeResult& result,
    const std::shared_ptr<impala::TRuntimeProfileTree>& tree);

};  // namespace strategies

#endif  // EXTERNAL_D2_STRATEGIES_H
