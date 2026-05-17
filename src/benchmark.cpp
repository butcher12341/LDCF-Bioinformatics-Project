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

BenchmarkResult BenchmarkLDCF(const std::vector<std::string>& kmers,
                               const std::vector<std::string>& negative_kmers,
                               size_t base_capacity) {
  BenchmarkResult result{};
  result.num_items = kmers.size();
  LDCF filter(base_capacity);

  auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& kmer : kmers) filter.Insert(kmer);
  auto t1 = std::chrono::high_resolution_clock::now();
  result.insert_time_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

  result.memory_bytes = GetMemoryUsageBytes();

  auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& kmer : kmers) filter.Contains(kmer);
  auto t3 = std::chrono::high_resolution_clock::now();
  result.lookup_time_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

  if (!negative_kmers.empty()) {
    size_t fp = 0;
    for (const auto& kmer : negative_kmers)
      if (filter.Contains(kmer)) fp++;
    result.false_positive_rate = static_cast<double>(fp) / negative_kmers.size();
  }

  return result;
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