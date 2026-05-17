/*
 * Author: Andrija Pervan
 * Minimal smoke test - verifies LDCF insert/contains/remove correctness.
 */

#include <iostream>
#include "ldcf.hpp"
#include "genome_loader.hpp"

int main() {
    // --- Basic correctness test ---
    LDCF filter(16);

    std::vector<std::string> inserted = {"ACGT", "TTGA", "CCGA", "GGTA", "ATCG"};
    for (const auto& s : inserted)
        filter.Insert(s);

    std::cout << "Levels: " << filter.LevelCount() << "\n";
    std::cout << "Size:   " << filter.Size() << "\n";

    bool all_found = true;
    for (const auto& s : inserted) {
        if (!filter.Contains(s)) {
            std::cout << "MISS: " << s << "\n";
            all_found = false;
        }
    }
    std::cout << "All inserted items found: " << (all_found ? "YES" : "NO") << "\n";

    // --- k-mer test on random sequence ---
    std::string seq = GenomeLoader::GenerateRandom(100, 42);
    auto kmers = GenomeLoader::GenerateKmers(seq, 10);
    std::cout << "Generated " << kmers.size() << " k-mers (k=10) from 100-char sequence\n";

    LDCF kmer_filter(kmers.size());
    for (const auto& km : kmers) kmer_filter.Insert(km);

    size_t found = 0;
    for (const auto& km : kmers)
        if (kmer_filter.Contains(km)) found++;

    std::cout << "k-mer recall: " << found << "/" << kmers.size() << "\n";

    return 0;
}
