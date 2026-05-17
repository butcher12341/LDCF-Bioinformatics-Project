#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include "ldcf.hpp"
#include "cuckoo_filter.hpp"
#include <string>
#include <vector>

// Author: Andrija Pervan

struct BenchmarkResult {
  double insert_time_ms;
  double lookup_time_ms;
  double false_positive_rate;
  size_t memory_bytes;
  size_t num_items;
};

// benchmarks the LDCF implementation using the same k-mer set and negative samples.
// negative_kmers = items not inserted, used to calculate false positive rate
BenchmarkResult BenchmarkLDCF(const std::vector<std::string>& kmers,
                               const std::vector<std::string>& negative_kmers,
                               size_t base_capacity);
// benchmarks the original CuckooFilter implementation using the same k-mer set and negative samples.
BenchmarkResult BenchmarkCF(const std::vector<std::string>& kmers,
                             const std::vector<std::string>& negative_kmers,
                             size_t capacity);

// reads VmRSS from /proc/self/status (Linux only)
size_t GetMemoryUsageBytes();

// appends one result row to a CSV file
void WriteResultCSV(const std::string& filepath, const std::string& label,
                    int k, const BenchmarkResult& result);

#endif  // BENCHMARK_HPP
