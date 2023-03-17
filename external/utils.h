#ifndef EXTERNAL_UTILS_H
#define EXTERNAL_UTILS_H
#include <memory>
#include <chrono>
#include "RuntimeProfile_types.h"


std::string Base64Decode(const char* const message, const size_t mlen);
std::shared_ptr<impala::TRuntimeProfileTree> DecodeProfile(const std::string& profile);


template<typename IT0, typename IT1>
bool StartsWith(IT0 start0, IT0 end0, IT1 start1, IT1 end1) {
  bool retval = false;
  for (;start0 != end0 && start1 != end1;++start0, ++start1) {
    if (*start0 != *start1)
      return false;
  }
  return start1 == end1;
}


int64_t ns2ms(int64_t ns);


std::chrono::system_clock::time_point DateTimeString2Chrono(const std::string& datetime);


#endif // EXTERNAL_UTILS_H
