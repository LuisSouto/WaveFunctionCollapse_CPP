#pragma once

#include "wfc_typedefs.h"
#include <adjacency_data.h>
#include <cstddef>
#include <sprite_holder.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

/*Looks for patterns in a sample image using overlapping.*/
class OverlappingPatterns {
private:
  int N;
  size_t width;
  size_t height;
  std::vector<pattern_hash_t> grid_pattern_hashes;
  std::unordered_map<pattern_hash_t, pattern_id_t> hashes_to_ids;
  std::vector<pattern_id_t> grid_pattern_ids;
  AdjacencyData adjacent_data;
  void computePatternHashes(const SpriteHolder &sprite, int N);
  void mapHashesToIds();
  void computeGridIds();
  void populateAdjacentData();

public:
  OverlappingPatterns(const SpriteHolder &sprite, int N);
};