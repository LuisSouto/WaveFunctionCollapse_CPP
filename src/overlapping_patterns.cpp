#include <cstddef>
#include <cstdint>
#include <directions.h>
#include <hash_boost.h>
#include <overlapping_patterns.h>
#include <unordered_map>
#include <vector>

OverlappingPatterns::OverlappingPatterns(const SpriteHolder &sprite, int N) {
  computePatterns(sprite, N);
}

// TODO: for now we do not use rotations, mirroring, wrapping, or any other
// pattern augmentation
void OverlappingPatterns::computePatterns(const SpriteHolder &sprite, int N) {
  const size_t width = sprite.getWidth() - N + 1;
  const size_t height = sprite.getHeight() - N + 1;
  std::vector<size_t> stored_patterns(width * height);
  std::unordered_map<int,
                     std::unordered_map<uint8_t, std::unordered_map<int, int>>>
      adjacent_patterns;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      size_t hash = 0;
      std::vector<uint32_t> pattern_pixel_hashes;
      for (int dy = 0; dy < N; dy++) {
        for (int dx = 0; dx < N; dx++) {
          const uint32_t pixelHash = sprite.getPixelHash(x + dx, y + dy);
          hash_combine(hash, pixelHash);
          pattern_pixel_hashes.push_back(pixelHash);
        }
      }
      stored_patterns[y * width + x] = hash;
      if (pattern_hashes.find(hash) == pattern_hashes.end()) {
        pattern_hashes[hash] = pattern_pixel_hashes;
        hashes_to_ids[hash] = hashes_to_ids.size();
      }
      if (x > 0) {
        size_t left_id = hashes_to_ids[stored_patterns[y * width + (x - 1)]];
        adjacent_patterns[left_id][Directions::RIGHT][hash]++;
        size_t right_id = hashes_to_ids[hash];
        adjacent_patterns[right_id][Directions::LEFT][left_id]++;
      }
      if (y > 0) {
        size_t top_id = hashes_to_ids[stored_patterns[(y - 1) * width + x]];
        adjacent_patterns[top_id][Directions::DOWN][hash]++;
        size_t bottom_id = hashes_to_ids[hash];
        adjacent_patterns[bottom_id][Directions::UP][top_id]++;
      }
      // TODO: for proper cache locality we need to flatten this 3D object
    }
  }
}