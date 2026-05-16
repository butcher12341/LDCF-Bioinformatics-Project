// Author: Andrija Pervan
// LDCF implementation, see ldcf.hpp

#include "ldcf.hpp"

LDCF::LDCF(size_t base_capacity) : base_capacity_(base_capacity) {
  levels_.push_back(new CuckooFilter(LevelCapacity(0)));
}

LDCF::~LDCF() {
  for (CuckooFilter* cf : levels_) delete cf;
}

size_t LDCF::LevelCapacity(size_t index) const {
  return base_capacity_ * (static_cast<size_t>(1) << index);
}

bool LDCF::Insert(const std::string& item) {
  for (size_t i = 0; i < levels_.size(); i++) {
    if (levels_[i]->insert(item)) {
      items_.push_back(item);
      return true;
    }
    // level full, try merging into next level then insert into freed space
    if (MergeUp(i)) {
      if (levels_[i]->insert(item)) {
        items_.push_back(item);
        return true;
      }
    }
  }

  // all levels full, add a new one
  size_t new_index = levels_.size();
  levels_.push_back(new CuckooFilter(LevelCapacity(new_index)));
  if (levels_[new_index]->insert(item)) {
    items_.push_back(item);
    return true;
  }

  return false;
}

bool LDCF::Contains(const std::string& item) const {
  for (const CuckooFilter* cf : levels_)
    if (cf->contains(item)) return true;
  return false;
}

bool LDCF::Remove(const std::string& item) {
  for (CuckooFilter* cf : levels_) {
    if (cf->remove(item)) {
      for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (*it == item) { items_.erase(it); break; }
      }
      return true;
    }
  }
  return false;
}

bool LDCF::MergeUp(size_t index) {
  if (index + 1 >= levels_.size())
    levels_.push_back(new CuckooFilter(LevelCapacity(index + 1)));

  CuckooFilter* src = levels_[index];
  CuckooFilter* dst = levels_[index + 1];

  // collect items that are in src (contains() may have false positives,
  // but that just means a few extra re-inserts, not incorrect behaviour)
  std::vector<std::string> to_move;
  for (const std::string& item : items_)
    if (src->contains(item)) to_move.push_back(item);

  for (const std::string& item : to_move) {
    if (!dst->insert(item)) {
      // dst also full, roll back and give up
      for (const std::string& ins : to_move) {
        if (ins == item) break;
        dst->remove(ins);
      }
      return false;
    }
  }

  // merge done, reset src
  delete levels_[index];
  levels_[index] = new CuckooFilter(LevelCapacity(index));
  return true;
}

size_t LDCF::Size() const { return items_.size(); }

size_t LDCF::Capacity() const {
  size_t total = 0;
  for (const CuckooFilter* cf : levels_) total += cf->getCapacity();
  return total;
}

double LDCF::LoadFactor() const {
  size_t cap = Capacity();
  return cap == 0 ? 0.0 : static_cast<double>(Size()) / static_cast<double>(cap);
}
