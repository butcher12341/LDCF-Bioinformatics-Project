// Authors: Andrija Pervan & Andrija Maček
// Main entry point - parses CLI arguments and runs benchmarks.

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <map>
#include <unordered_set>
#include "ldcf.hpp"
#include "genome_loader.hpp"
#include "benchmark.hpp"

void printUsage(const char* prog) {
  std::cerr << "Usage: " << prog << " [options]\n"
            << "Options:\n"
            << "  --fasta <path>     Path to FASTA file (E. coli genome)\n"
            << "  --random <len>     Generate random sequence of given length\n"
            << "  --k <values>       Comma-separated k values (default: 10,20,50,100,200)\n"
            << "  --capacity <n>     CF block capacity (default: 1024)\n"
            << "  --output <path>    Output CSV file (default: results.csv)\n"
            << "  --config <path>    Load config data from a file instead of a CLI (example: data/config.txt)"
            << "  --help             Show this message\n";
}

// parse comma-separated integers
std::vector<int> parseKValues(const std::string& str) {
  std::vector<int> vals;
  size_t start = 0;
  while (start < str.size()) {
    size_t end = str.find(',', start);
    if (end == std::string::npos) end = str.size();
    vals.push_back(std::stoi(str.substr(start, end - start)));
    start = end + 1;
  }
  return vals;
}

std::map<std::string, std::string> LoadConfig(const std::string& path) {
  std::map<std::string, std::string> config;
  std::ifstream in(path);
  if (!in)
    return config;

  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    size_t eq = line.find('=');
    if (eq == std::string::npos)
      continue;

    std::string key = line.substr(0, eq);
    std::string value = line.substr(eq + 1);
    config[key] = value;
  }

  return config;
}

int main(int argc, char* argv[]) {
  std::string config_path;
  std::string fasta_path;
  size_t random_len = 0;
  std::vector<int> k_values = {10, 20, 50, 100, 200};
  size_t capacity = 1024;
  std::string output_path = "results.csv";

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--fasta" && i + 1 < argc) fasta_path = argv[++i];
    else if (arg == "--random" && i + 1 < argc) random_len = std::stoull(argv[++i]);
    else if (arg == "--k" && i + 1 < argc) k_values = parseKValues(argv[++i]);
    else if (arg == "--capacity" && i + 1 < argc) capacity = std::stoull(argv[++i]);
    else if (arg == "--output" && i + 1 < argc) output_path = argv[++i];
    else if (arg == "--config" && i + 1 < argc) config_path = argv[++i];
    else if (arg == "--help") { printUsage(argv[0]); return 0; }
    else { std::cerr << "Unknown option: " << arg << "\n"; printUsage(argv[0]); return 1; }
  }

  if (!config_path.empty()) {
    std::map<std::string, std::string> cfg = LoadConfig(config_path);
    if (cfg.count("fasta"))
      fasta_path = cfg["fasta"];
    if (cfg.count("random"))
      random_len = std::stoull(cfg["random"]);
    if (cfg.count("k"))
      k_values = parseKValues(cfg["k"]);
    if (cfg.count("capacity"))
      capacity = std::stoull(cfg["capacity"]);
    if (cfg.count("output"))
      output_path = cfg["output"];
  }

  if (fasta_path.empty() && random_len == 0) {
    std::cerr << "Error: specify --fasta or --random\n";
    printUsage(argv[0]);
    return 1;
  }

  // load or generate sequence
  std::string sequence;
  std::string source_label;
  if (!fasta_path.empty()) {
    std::cout << "Loading FASTA: " << fasta_path << "\n";
    sequence = GenomeLoader::LoadFasta(fasta_path);
    source_label = "ecoli";
  } else {
    std::cout << "Generating random sequence, length=" << random_len << "\n";
    sequence = GenomeLoader::GenerateRandom(random_len);
    source_label = "random_" + std::to_string(random_len);
  }
  std::cout << "Sequence length: " << sequence.size() << "\n\n";

  // generate negative samples (different seed) for FPR measurement
  std::string neg_sequence = GenomeLoader::GenerateRandom(sequence.size(), 99);

  for (int k : k_values) {
    if (static_cast<size_t>(k) > sequence.size()) {
      std::cout << "Skipping k=" << k << " (larger than sequence)\n";
      continue;
    }

    auto kmers = GenomeLoader::GenerateKmers(sequence, k);
    auto neg_kmers_raw = GenomeLoader::GenerateKmers(neg_sequence, k);

    // filter out negative k-mers that exist in the positive set
    std::unordered_set<std::string> pos_set(kmers.begin(), kmers.end());
    std::vector<std::string> neg_kmers;
    neg_kmers.reserve(neg_kmers_raw.size());
    for (auto& nk : neg_kmers_raw)
      if (pos_set.find(nk) == pos_set.end()) neg_kmers.push_back(std::move(nk));
    neg_kmers_raw.clear();
    pos_set.clear();

    std::cout << "k=" << k << " | kmers=" << kmers.size()
              << " | neg_kmers=" << neg_kmers.size() << "\n";

    // benchmark LDCF
    auto ldcf_result = BenchmarkLDCF(kmers, neg_kmers, capacity);
    WriteResultCSV(output_path, "LDCF_" + source_label, k, ldcf_result);
    std::cout << "  LDCF: insert=" << ldcf_result.insert_time_ms << "ms"
              << " lookup=" << ldcf_result.lookup_time_ms << "ms"
              << " FPR=" << ldcf_result.false_positive_rate << "\n";

    // benchmark plain CF for comparison
    auto cf_result = BenchmarkCF(kmers, neg_kmers, kmers.size());
    WriteResultCSV(output_path, "CF_" + source_label, k, cf_result);
    std::cout << "  CF:   insert=" << cf_result.insert_time_ms << "ms"
              << " lookup=" << cf_result.lookup_time_ms << "ms"
              << " FPR=" << cf_result.false_positive_rate << "\n\n";
  }

  std::cout << "Results written to: " << output_path << "\n";
  return 0;
}
