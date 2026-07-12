#include "wfc_typedefs.h"
#include <cstddef>
#include <cstdint>
#include <directions.h>
#include <hash_boost.h>
#include <overlapping_patterns.h>
#include <unordered_map>
#include <vector>

OverlappingPatterns::OverlappingPatterns(const SpriteHolder &sprite, int N) {
  computePatternHashes(sprite, N);
  mapHashesToIds();
  computeGridIds();
  countPatterns();
  populateAdjacentData();
}

void OverlappingPatterns::computePatternHashes(const SpriteHolder &sprite,
                                               int N) {

  this->N = N;
  width = sprite.getWidth() - N + 1;
  height = sprite.getHeight() - N + 1;
  grid_pattern_hashes.resize(width * height);
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      pattern_hash_t pattern_hash = 0;
      for (size_t dy = 0; dy < N; dy++) {
        for (size_t dx = 0; dx < N; dx++) {
          const pixel_hash_t pixelHash = sprite.getPixelHash(x + dx, y + dy);
          pattern_hash = hash_combine(pattern_hash, pixelHash);
        }
      }
      grid_pattern_hashes[y * width + x] = pattern_hash;
    }
  }
}

void OverlappingPatterns::mapHashesToIds() {
  for (pattern_hash_t pattern_hash : grid_pattern_hashes) {
    if (!hashes_to_ids.contains(pattern_hash)) {
      hashes_to_ids[pattern_hash] =
          static_cast<pattern_id_t>(hashes_to_ids.size());
    }
  }
}

void OverlappingPatterns::computeGridIds() {
  grid_pattern_ids.clear();
  grid_pattern_ids.reserve(grid_pattern_hashes.size());
  for (pattern_hash_t pattern_hash : grid_pattern_hashes) {
    grid_pattern_ids.push_back(hashes_to_ids[pattern_hash]);
  }
}

void OverlappingPatterns::countPatterns() {
  pattern_frequencies.clear();
  pattern_frequencies.resize(hashes_to_ids.size(), 0);
  for (pattern_id_t pattern_id : grid_pattern_ids) {
    pattern_frequencies[pattern_id]++;
  }
}

// TODO: for now we do not use rotations, mirroring, wrapping, or any other
// pattern augmentation
void OverlappingPatterns::populateAdjacentData() {
  // DISCOVERY PHASE: look for adjacent patterns in the grid and count their
  // frequencies
  std::vector<std::unordered_map<pattern_id_t, uint64_t>> adjacent_patterns(
      hashes_to_ids.size() * NUM_DIRECTIONS_2D);
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      if (x > 0) {
        pattern_id_t left_id = grid_pattern_ids[y * width + (x - 1)];
        pattern_id_t right_id = grid_pattern_ids[y * width + x];
        adjacent_patterns[left_id * NUM_DIRECTIONS_2D + Directions::RIGHT]
                         [right_id]++;
        adjacent_patterns[right_id * NUM_DIRECTIONS_2D + Directions::LEFT]
                         [left_id]++;
      }
      if (y > 0) {
        size_t top_id = grid_pattern_ids[(y - 1) * width + x];
        size_t bottom_id = grid_pattern_ids[y * width + x];
        adjacent_patterns[top_id * NUM_DIRECTIONS_2D + Directions::DOWN]
                         [bottom_id]++;
        adjacent_patterns[bottom_id * NUM_DIRECTIONS_2D + Directions::UP]
                         [top_id]++;
      }
    }
  }

  adjacent_data = AdjacencyData(adjacent_patterns, pattern_frequencies);
  // POPULATION PHASE: populate the AdjacencyData structure with the discovered
  // adjacent patterns and their frequencies
}
