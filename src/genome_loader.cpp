/*
 * Author: Andrija Pervan
 *
 * See genome_loader.hpp for description.
 */

#include "genome_loader.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdlib>

std::string GenomeLoader::LoadFasta(const std::string& filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filepath);
  }

  std::string sequence;
  std::string line;
  while (std::getline(file, line)) {
    // skip header lines starting with '>'
    if (line.empty() || line[0] == '>') continue;
    // remove carriage return if present (Windows line endings)
    if (!line.empty() && line.back() == '\r') line.pop_back();
    sequence += line;
  }
  return sequence;
}

std::vector<std::string> GenomeLoader::GenerateKmers(
    const std::string& sequence, int k) {
  std::vector<std::string> kmers;
  if (k <= 0 || static_cast<size_t>(k) > sequence.size()) return kmers;

  kmers.reserve(sequence.size() - k + 1);
  for (size_t i = 0; i <= sequence.size() - k; i++) {
    kmers.push_back(sequence.substr(i, k));
  }
  return kmers;
}

std::string GenomeLoader::GenerateRandom(size_t length, unsigned int seed) {
  static const char bases[] = "ACGT";
  std::srand(seed);
  std::string seq;
  seq.reserve(length);
  for (size_t i = 0; i < length; i++) {
    seq += bases[std::rand() % 4];
  }
  return seq;
}
