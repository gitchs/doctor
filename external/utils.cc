#include <memory>
#include <chrono>
#include <sstream>
#include "ostream"
#include "boost/archive/iterators/transform_width.hpp"
#include "boost/archive/iterators/binary_from_base64.hpp"
#include "RuntimeProfile_types.h"
#include "thrift/protocol/TBase64Utils.h"
#include "thrift/protocol/TCompactProtocol.h"
#include "thrift/protocol/TDebugProtocol.h"
#include "thrift/protocol/TJSONProtocol.h"
#include "thrift/protocol/TProtocol.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/TZlibTransport.h"

#include "utils.h"

extern "C" {
// CentOS 7 gcc version is 4.8.5
// When gcc <= 5.0, it does not implement std::get_time,
// so we need to use the C API strptime
#include <time.h>
}

std::string Base64Decode(const char* const message, const size_t mlen) {
  std::stringstream ss;
  typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<const char*>, 8, 6> Base64Decoder;
  std::copy(Base64Decoder(message), Base64Decoder(message + mlen), std::ostream_iterator<char>(ss));
  return ss.str();
}


std::shared_ptr<impala::TRuntimeProfileTree> DecodeProfile(const std::string& profile) {

  size_t plen = profile.size() * 3 / 4;

  std::string raw_profile = Base64Decode(profile.c_str(), profile.size());

  std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer =
      std::make_shared<apache::thrift::transport::TMemoryBuffer>(plen);
  buffer->write((unsigned char*)raw_profile.c_str(), raw_profile.size());
  apache::thrift::protocol::TCompactProtocol protocol(
      apache::thrift::transport::TZlibTransportFactory().getTransport(buffer));
  std::shared_ptr<impala::TRuntimeProfileTree> tree = std::make_shared<impala::TRuntimeProfileTree>();
  tree->read(&protocol);
  return tree;
}

int64_t ns2ms(int64_t ns) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(ns)).count();
}


std::chrono::system_clock::time_point DateTimeString2Chrono(const std::string& datetime) {
  // input sample: 2023-03-14 00:00:00.692019000
  std::chrono::system_clock::time_point retval;
  struct tm tm;
  strptime(datetime.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
  time_t ts = mktime(&tm);
  return std::chrono::system_clock::from_time_t(ts);
}

