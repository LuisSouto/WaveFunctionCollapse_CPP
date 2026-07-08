
/*Looks for patterns in a sampel image using overlapping.*/
#ifndef OVERLAPPING_PATTERNS_H
#define OVERLAPPING_PATTERNS_H

#include <cstddef>
#include <sprite_holder.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

class OverlappingPatterns {
private:
  std::unordered_map<size_t, std::vector<u_int32_t>> pattern_hashes;
  std::unordered_map<size_t, int> hashes_to_ids;
  void computePatterns(const SpriteHolder &sprite, int N);

public:
  OverlappingPatterns(const SpriteHolder &sprite, int N);
  const std::unordered_map<size_t, std::vector<u_int32_t>> &
  getPatternHashes() const {
    return pattern_hashes;
  }
  const std::unordered_map<size_t, int> &getHashesToIds() const {
    return hashes_to_ids;
  }
};

#endif // OVERLAPPING_PATTERNS_H