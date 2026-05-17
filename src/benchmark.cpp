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

void WriteResultCSV(const std::string& filepath, const std::string& label,
                    int k, const BenchmarkResult& result) {
  std::ofstream file(filepath, std::ios::app);
  if (!file.is_open())
    throw std::runtime_error("Cannot open output file: " + filepath);

  if (file.tellp() == 0)
    file << "filter,k,num_items,insert_ms,lookup_ms,fpr,memory_bytes\n";

  file << label << "," << k << "," << result.num_items << ","
       << result.insert_time_ms << "," << result.lookup_time_ms << ","
       << result.false_positive_rate << "," << result.memory_bytes << "\n";
}