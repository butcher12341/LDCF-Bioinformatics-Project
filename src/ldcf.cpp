// Author: Andrija Pervan
// LDCF implementation using binary tree of CF blocks with fingerprint-prefix splitting.

#include "ldcf.hpp"
#include <openssl/md5.h>

// --- LDCFNode ---

LDCFNode::LDCFNode(size_t capacity, int depth, uint32_t prefix)
    : cf(new CuckooFilter(capacity)), depth(depth), prefix(prefix),
      left(nullptr), right(nullptr) {}

LDCFNode::~LDCFNode() {
  delete cf;
  delete left;
  delete right;
}

// --- LDCF ---

LDCF::LDCF(size_t block_capacity)
    : block_capacity_(block_capacity), size_(0),
      root_(new LDCFNode(block_capacity, 0, 0)) {}

LDCF::~LDCF() { delete root_; }

int LDCF::GetPrefixBit(uint32_t fingerprint, int bit_pos) {
  return (fingerprint >> bit_pos) & 1;
}

LDCFNode* LDCF::FindLeaf(uint32_t fingerprint) const {
  LDCFNode* node = root_;
  while (!node->IsLeaf()) {
    if (GetPrefixBit(fingerprint, node->depth) == 0)
      node = node->left;
    else
      node = node->right;
  }
  return node;
}

bool LDCF::Insert(const std::string& item) {
  uint32_t fp = CuckooFilter::getFingerprint(item);
  LDCFNode* leaf = FindLeaf(fp);

  if (leaf->cf->insert(item)) {
    size_++;
    return true;
  }

  // CF block is full, split and retry
  if (SplitNode(leaf, item)) {
    size_++;
    return true;
  }

  return false;
}

bool LDCF::Contains(const std::string& item) const {
  uint32_t fp = CuckooFilter::getFingerprint(item);
  LDCFNode* leaf = FindLeaf(fp);
  return leaf->cf->contains(item);
}

bool LDCF::Remove(const std::string& item) {
  uint32_t fp = CuckooFilter::getFingerprint(item);
  LDCFNode* leaf = FindLeaf(fp);
  if (leaf->cf->remove(item)) {
    size_--;
    return true;
  }
  return false;
}

bool LDCF::SplitNode(LDCFNode* node, const std::string& new_item) {
  if (node->depth >= 15) return false;  // 16-bit fingerprint, max 15 splits

  int split_bit = node->depth;
  uint32_t left_prefix = node->prefix;
  uint32_t right_prefix = node->prefix | (1 << split_bit);

  LDCFNode* left_child = new LDCFNode(block_capacity_, split_bit + 1, left_prefix);
  LDCFNode* right_child = new LDCFNode(block_capacity_, split_bit + 1, right_prefix);

  // redistribute existing fingerprints from old CF into children
  // iterate all buckets and move each fingerprint to the correct child
  const auto& table = node->cf->getTable();
  size_t bucket_cnt = node->cf->getBucketCount();
  bool success = true;

  for (size_t b = 0; b < bucket_cnt && success; b++) {
    for (uint32_t fp : table[b].bucket) {
      LDCFNode* target = (GetPrefixBit(fp, split_bit) == 0) ? left_child : right_child;
      if (!target->cf->insert(fp, b)) {
        success = false;
        break;
      }
    }
  }

  if (!success) {
    // split failed (shouldn't happen normally since we're halving the load)
    delete left_child;
    delete right_child;
    return false;
  }

  // insert the new item into the correct child
  uint32_t new_fp = CuckooFilter::getFingerprint(new_item);
  LDCFNode* target = (GetPrefixBit(new_fp, split_bit) == 0) ? left_child : right_child;
  if (!target->cf->insert(new_item)) {
    delete left_child;
    delete right_child;
    return false;
  }

  // split succeeded, replace node's CF with children
  delete node->cf;
  node->cf = nullptr;
  node->left = left_child;
  node->right = right_child;

  return true;
}

size_t LDCF::NodeCount() const { return CountNodes(root_); }

size_t LDCF::CountNodes(const LDCFNode* node) {
  if (!node) return 0;
  if (node->IsLeaf()) return 1;
  return CountNodes(node->left) + CountNodes(node->right);
}

size_t LDCF::Capacity() const { return SumCapacity(root_); }

size_t LDCF::SumCapacity(const LDCFNode* node) {
  if (!node) return 0;
  if (node->IsLeaf()) return node->cf->getCapacity();
  return SumCapacity(node->left) + SumCapacity(node->right);
}

double LDCF::LoadFactor() const {
  size_t cap = Capacity();
  return cap == 0 ? 0.0 : static_cast<double>(size_) / static_cast<double>(cap);
}
