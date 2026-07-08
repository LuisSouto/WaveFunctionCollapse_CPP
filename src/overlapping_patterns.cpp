#include <cstdint>
#include <hash_boost.h>
#include <overlapping_patterns.h>
#include <vector>

OverlappingPatterns::OverlappingPatterns(const SpriteHolder &sprite, int N) {
  computePatternHashes(sprite, N);
}

// TODO: for now we do not use rotations, mirroring, wrapping, or any other
// pattern augmentation
void OverlappingPatterns::computePatternHashes(const SpriteHolder &sprite,
                                               int N) {
  const size_t width = sprite.getWidth() - N + 1;
  const size_t height = sprite.getHeight() - N + 1;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      size_t hash = 0;
      std::vector<uint32_t> pattern_pixel_hashes;
      for (int dx = 0; dx < N; dx++) {
        for (int dy = 0; dy < N; dy++) {
          const uint32_t pixelHash = sprite.getPixelHash(x + dx, y + dy);
          hash_combine(hash, pixelHash);
          pattern_pixel_hashes.push_back(pixelHash);
        }
      }
      pattern_hashes[hash] = pattern_pixel_hashes;
      if (hashes_to_ids.find(hash) == hashes_to_ids.end()) {
        hashes_to_ids[hash] = hashes_to_ids.size();
      }
    }
  }
}