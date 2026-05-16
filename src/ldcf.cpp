/*
 * Author: [Član 2]
 *
 * Logarithmic Dynamic Cuckoo Filter (LDCF) implementation.
 * See ldcf.hpp for algorithm description.
 */

#include "ldcf.hpp"
#include <stdexcept>

LDCF::LDCF(size_t base_capacity) : base_capacity_(base_capacity) {
  // Start with a single level-0 filter
  levels_.push_back(new CuckooFilter(LevelCapacity(0)));
}

LDCF::~LDCF() {
  for (CuckooFilter* cf : levels_) {
    delete cf;
  }
  levels_.clear();
}

size_t LDCF::LevelCapacity(size_t index) const {
  // Each level is 2^index times the base capacity
  return base_capacity_ * (static_cast<size_t>(1) << index);
}

bool LDCF::Insert(const std::string& item) {
  // Try inserting into each existing level in order
  for (size_t i = 0; i < levels_.size(); i++) {
    if (levels_[i]->insert(item)) {
      items_.push_back(item);
      return true;
    }
    // Level i is full or cuckoo-looped — attempt merge into level i+1
    if (MergeUp(i)) {
      // After merge, level i is empty and level i+1 has the old contents.
      // Try inserting into level i (now empty).
      if (levels_[i]->insert(item)) {
        items_.push_back(item);
        return true;
      }
    }
  }

  // All existing levels exhausted — create a new level and try again
  size_t new_index = levels_.size();
  levels_.push_back(new CuckooFilter(LevelCapacity(new_index)));
  if (levels_[new_index]->insert(item)) {
    items_.push_back(item);
    return true;
  }

  return false;
}

bool LDCF::Contains(const std::string& item) const {
  // An item may be in any active level
  for (const CuckooFilter* cf : levels_) {
    if (cf->contains(item)) {
      return true;
    }
  }
  return false;
}

bool LDCF::Remove(const std::string& item) {
  // Remove from the first level that reports it present
  for (CuckooFilter* cf : levels_) {
    if (cf->remove(item)) {
      // Also remove from items_ tracking vector
      for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (*it == item) {
          items_.erase(it);
          break;
        }
      }
      return true;
    }
  }
  return false;
}

bool LDCF::MergeUp(size_t index) {
  // Ensure the target level exists
  if (index + 1 >= levels_.size()) {
    levels_.push_back(new CuckooFilter(LevelCapacity(index + 1)));
  }

  CuckooFilter* src = levels_[index];
  CuckooFilter* dst = levels_[index + 1];

  // Re-insert all tracked items that belong to src into dst.
  // We identify them by querying src (fingerprint-based, may have false
  // positives, but that only causes unnecessary re-inserts, not data loss).
  std::vector<std::string> to_move;
  for (const std::string& item : items_) {
    if (src->contains(item)) {
      to_move.push_back(item);
    }
  }

  // Attempt to insert all candidates into dst
  for (const std::string& item : to_move) {
    if (!dst->insert(item)) {
      // dst is also full — merge cannot complete, leave state unchanged
      // Roll back: remove items already inserted into dst this round
      for (const std::string& inserted : to_move) {
        if (inserted == item) break;
        dst->remove(inserted);
      }
      return false;
    }
  }

  // Merge succeeded — clear src
  src->clearFilter();
  // Re-initialise src as an empty filter of the same capacity
  delete levels_[index];
  levels_[index] = new CuckooFilter(LevelCapacity(index));

  return true;
}

size_t LDCF::Size() const {
  return items_.size();
}

size_t LDCF::Capacity() const {
  size_t total = 0;
  for (const CuckooFilter* cf : levels_) {
    total += cf->getCapacity();
  }
  return total;
}

double LDCF::LoadFactor() const {
  size_t cap = Capacity();
  if (cap == 0) return 0.0;
  return static_cast<double>(Size()) / static_cast<double>(cap);
}
