# Logarithmic Dynamic Cuckoo Filter (LDCF)

Implementation of the Logarithmic Dynamic Cuckoo Filter based on Zhang et al. (2021).

> Zhang et al. "The Logarithmic Dynamic Cuckoo Filter", IEEE ICDE 2021, doi: 10.1109/ICDE51399.2021.00087

Project for the course Bioinformatics 1, 2025/2026 at FER, University of Zagreb.

Course page: https://www.fer.unizg.hr/predmet/bio1

## Team

- Andrija Macek
- Andrija Pervan

## Requirements

- Linux (tested on Ubuntu 20.04)
- CMake >= 3.16
- g++ with C++17 support
- OpenSSL development library

Install dependencies:
```bash
sudo apt-get install cmake g++ libssl-dev
```

## Build

```bash
cmake -B build -S .
cmake --build build
```

## Usage

```bash
./build/ldcf [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--fasta <path>` | Path to a FASTA file (e.g. E. coli genome) |
| `--random <length>` | Generate a random DNA sequence of given length |
| `--k <values>` | Comma-separated k-mer sizes (default: 10,20,50,100,200) |
| `--capacity <n>` | CF block capacity (default: 1024) |
| `--output <path>` | Output CSV file path (default: results.csv) |
| `--help` | Show help message |

### Examples

Quick test (runs in seconds):
```bash
./build/ldcf --random 1000 --k 10,20,50 --output results/quick_test.csv
```

Benchmark on E. coli genome:
```bash
wget -O data/ecoli.fasta "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nucleotide&id=NC_000913.3&rettype=fasta&retmode=text"
./build/ldcf --fasta data/ecoli.fasta --k 10,20,50,100,200 --capacity 4096 --output results/ecoli_results.csv
```

Benchmark on synthetic data (10^6 characters):
```bash
./build/ldcf --random 1000000 --k 10,20,50,100,200 --capacity 4096 --output results/synthetic_results.csv
```

## Output

Results are written to a CSV file with columns:
```
filter,k,num_items,insert_ms,lookup_ms,fpr,memory_bytes
```

## License

MIT
