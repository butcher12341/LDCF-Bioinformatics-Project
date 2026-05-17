// Author: Andrija Pervan
// Benchmark implementation, see benchmark.hpp

#include "benchmark.hpp"
#include <chrono>
#include <fstream>
#include <stdexcept>

size_t GetMemoryUsageBytes() {
#ifdef __linux__
  std::ifstream status("/proc/self/status");
  std::string line;
  while (std::getline(status, line)) {
    if (line.rfind("VmRSS:", 0) == 0)
      return std::stoull(line.substr(6)) * 1024;
  }
#endif
  return 0;
}