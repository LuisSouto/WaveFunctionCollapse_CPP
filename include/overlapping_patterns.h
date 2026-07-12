#pragma once

#include "wfc_typedefs.h"
#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
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
  std::unordered_map<pattern_hash_t, std::vector<uint8_t>> hashes_to_pixels;
  std::unordered_map<pattern_hash_t, pattern_id_t> hashes_to_ids;
  std::vector<pattern_id_t> grid_pattern_ids;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<uint8_t> ids_to_pixels;
  AdjacencyData adjacent_data;
  void computePatternHashes(const SpriteHolder &sprite, int N);
  void mapHashesToIds();
  void computeGridIds();
  void countPatterns();
  void populateAdjacentData();
  void mapIdsToPixels();

public:
  OverlappingPatterns(const SpriteHolder &sprite, int N);
  AdjacencyData getAdjacencyData() const { return adjacent_data; }
  std::vector<uint8_t> getIdsToPixels(std::span<const pattern_id_t> pattern_ids,
                                      size_t width, size_t height) const {
    std::vector<uint8_t> output_pixels;
    size_t output_width = width + N - 1;
    size_t output_height = height + N - 1;
    output_pixels.resize(output_width * output_height * 3);
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        pattern_id_t pattern_id = pattern_ids[y * width + x];
        const std::vector<uint8_t> &pixel_data =
            hashes_to_pixels.at(grid_pattern_hashes[pattern_id]);
        for (size_t dy = 0; dy < N; dy++) {
          for (size_t dx = 0; dx < N; dx++) {
            size_t pixel_index = (dy * N + dx) * 3;
            size_t output_x = x + dx;
            size_t output_y = y + dy;
            for (size_t i = 0; i < 3; i++) {
              output_pixels[(output_y * output_width + output_x) * 3 + i] =
                  pixel_data[pixel_index + i];
            }
          }
        }
      }
    }
    return output_pixels;
  }
};
