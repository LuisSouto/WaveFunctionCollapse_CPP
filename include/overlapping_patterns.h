#pragma once

#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <sprite_holder.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <wfc_settings.h>
#include <wfc_typedefs.h>

/*Looks for patterns in a sample image using overlapping.*/
class OverlappingPatterns {
private:
  std::vector<uint64_t> pattern_frequencies;
  std::vector<uint64_t> patterns_at_boundaries;
  std::vector<pattern_hash_t> grid_pattern_hashes;
  std::vector<pattern_id_t> grid_pattern_ids;
  std::vector<uint8_t> ids_to_pixels;
  std::unordered_map<pattern_hash_t, pattern_id_t> hashes_to_ids;
  WFCSettings settings;
  size_t N;
  size_t channels;
  size_t width;
  size_t height;
  size_t total_pixels;
  void computePatternHashes(const SpriteHolder &sprite, size_t N);
  void mapHashesToIds();
  void computeGridIds();
  void findBoundaryPatterns();
  void mapIdsToPixels(const SpriteHolder &sprite);
  void countPatterns();
  AdjacencyData generateAdjacentData() const;

public:
  OverlappingPatterns(const SpriteHolder &sprite, int N);

  AdjacencyData getAdjacencyData() const { return generateAdjacentData(); }

  size_t getNumPatterns() const { return hashes_to_ids.size(); }

  std::vector<uint8_t>
  convertIdsToPixels(std::span<const pattern_id_t> pattern_ids, size_t width,
                     size_t height) const;

  std::vector<uint8_t> getInputPixelPatterns() const {
    std::vector<pattern_id_t> ids(hashes_to_ids.size());
    std::iota(ids.begin(), ids.end(), 0);

    return convertIdsToPixels(ids, ids.size(), 1);
  }
};
