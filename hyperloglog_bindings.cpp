

#ifdef __EMSCRIPTEN__

#include "hyperloglog.hpp"
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(HyperLogLog_CPP) {
  using namespace emscripten;
  using T = std::string;
  using HyperLogLog = HLL::HyperLogLog<T>;
  class_<HyperLogLog>("HyperLogLog")
  .constructor<size_t, std::function<uint32_t(T)>>()
  .function("add", &HyperLogLog::add)
  .function("get_cardinality", &HyperLogLog::get_cardinality);
}
#endif