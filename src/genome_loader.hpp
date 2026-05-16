#ifndef GENOME_LOADER_HPP
#define GENOME_LOADER_HPP

#include <string>
#include <vector>

/*
 * Author: Andrija Pervan
 * Loads DNA sequences from FASTA files and generates k-mers.
 */
class GenomeLoader {
 public:
  // reads FASTA file, returns concatenated sequence (skips header lines)
  static std::string LoadFasta(const std::string& filepath);

  // sliding window k-mer generation, e.g. "ACGT" k=3 -> {"ACG","CGT"}
  static std::vector<std::string> GenerateKmers(const std::string& sequence, int k);

  // generates random DNA sequence (A/C/G/T) of given length, for testing
  static std::string GenerateRandom(size_t length, unsigned int seed = 42);
};

#endif  // GENOME_LOADER_HPP
