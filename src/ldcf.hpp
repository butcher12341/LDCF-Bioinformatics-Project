#ifndef LDCF_HPP
#define LDCF_HPP

#include "cuckoo_filter.hpp"
#include <vector>
#include <string>
#include <cstdint>

/*
 * Author: Andrija Pervan
 *
 * Logarithmic Dynamic Cuckoo Filter (LDCF)
 * Zhang et al., doi: 10.1109/ICDE51399.2021.00087
 */
class LDCF {
 public:
  explicit LDCF(size_t base_capacity);
  ~LDCF();

  // returns false only if expansion also fails
  bool Insert(const std::string& item);
  bool Contains(const std::string& item) const;
  // returns false if item not found
  bool Remove(const std::string& item);

  size_t Size() const;
  size_t Capacity() const;
  size_t LevelCount() const { return levels_.size(); }
  double LoadFactor() const;

 private:
  size_t base_capacity_;
  // need to track items separately since CF only stores fingerprints,
  // can't enumerate them during merge
  std::vector<std::string> items_;
  std::vector<CuckooFilter*> levels_;

  // tries to move all items from level[index] into level[index+1]
  bool MergeUp(size_t index);
  // capacity doubles with each level: base * 2^index
  size_t LevelCapacity(size_t index) const;
};

#endif  // LDCF_HPP
