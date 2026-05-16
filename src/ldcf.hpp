#ifndef LDCF_HPP
#define LDCF_HPP

#include "cuckoo_filter.hpp"
#include <vector>
#include <string>
#include <cstdint>

/*
 * Author: [Član 2]
 *
 * Logarithmic Dynamic Cuckoo Filter (LDCF)
 *
 * Based on: Zhang et al. "The Logarithmic Dynamic Cuckoo Filter"
 * doi: 10.1109/ICDE51399.2021.00087
 *
 * LDCF maintains a sequence of Cuckoo Filters at exponentially increasing
 * capacities (levels). When a level is full, its contents are merged into
 * the next (2x larger) level. This keeps the number of active levels at
 * O(log n) while supporting dynamic inserts and deletes.
 *
 * Level layout:
 *   level[0]: capacity = baseCapacity
 *   level[1]: capacity = 2 * baseCapacity
 *   level[2]: capacity = 4 * baseCapacity
 *   ...
 *   level[i]: capacity = 2^i * baseCapacity
 */
class LDCF {
 public:
  // Create LDCF with given base capacity for level 0
  explicit LDCF(size_t base_capacity);
  ~LDCF();

  // Insert item into the filter. Returns false only if insert is impossible
  // after exhausting all expansion attempts.
  bool Insert(const std::string& item);

  // Returns true if item is probably in the set, false if definitely not.
  bool Contains(const std::string& item) const;

  // Remove item from the filter. Returns false if item is not found.
  bool Remove(const std::string& item);

  // Returns total number of items currently stored across all levels
  size_t Size() const;

  // Returns total capacity across all active levels
  size_t Capacity() const;

  // Returns number of active levels
  size_t LevelCount() const { return levels_.size(); }

  // Returns overall load factor (size / capacity)
  double LoadFactor() const;

 private:
  size_t base_capacity_;
  // items_ tracks all inserted items for use during level merges,
  // since CuckooFilter stores only fingerprints and cannot enumerate them.
  std::vector<std::string> items_;
  std::vector<CuckooFilter*> levels_;

  // Attempt to merge level[index] into level[index+1].
  // Creates level[index+1] if it does not exist.
  // Returns true if all items were successfully re-inserted.
  bool MergeUp(size_t index);

  // Returns the capacity for a given level index: baseCapacity * 2^index
  size_t LevelCapacity(size_t index) const;
};

#endif  // LDCF_HPP
