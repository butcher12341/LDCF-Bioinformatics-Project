#ifndef LDCF_HPP
#define LDCF_HPP

#include "cuckoo_filter.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

/*
 * Author: Andrija Pervan
 *
 * Uses a binary tree of CF blocks. When a CF is full, it splits into
 * two children based on the fingerprint prefix. Lookup follows the
 * tree path determined by the item's fingerprint prefix bits, giving
 * O(log n) query time instead of scanning all CFs.
 */

// a single node in the LDCF binary tree
struct LDCFNode {
  CuckooFilter* cf;
  int depth;         // depth in tree = number of prefix bits used
  uint16_t prefix;   // fingerprint prefix that this node covers
  LDCFNode* left;    // child for prefix bit 0
  LDCFNode* right;   // child for prefix bit 1

  LDCFNode(size_t capacity, int depth, uint16_t prefix);
  ~LDCFNode();
  bool IsLeaf() const { return left == nullptr && right == nullptr; }
};

class LDCF {
 public:
  explicit LDCF(size_t block_capacity);
  ~LDCF();

  bool Insert(const std::string& item);
  bool Contains(const std::string& item) const;
  bool Remove(const std::string& item);

  size_t Size() const { return size_; }
  size_t NodeCount() const;
  size_t Capacity() const;
  double LoadFactor() const;

 private:
  size_t block_capacity_;  // capacity of each CF block
  size_t size_;
  LDCFNode* root_;

  // get fingerprint for an item (same hash as CuckooFilter uses)
  uint16_t GetFingerprint(const std::string& item) const;

  // extract the bit at position 'bit_pos' from fingerprint
  static int GetPrefixBit(uint16_t fingerprint, int bit_pos);

  // find the leaf node responsible for a given fingerprint
  LDCFNode* FindLeaf(uint16_t fingerprint) const;

  // split a full leaf node into two children
  bool SplitNode(LDCFNode* node, const std::string& item);

  // count nodes recursively
  static size_t CountNodes(const LDCFNode* node);
  static size_t SumCapacity(const LDCFNode* node);
};

#endif  // LDCF_HPP
