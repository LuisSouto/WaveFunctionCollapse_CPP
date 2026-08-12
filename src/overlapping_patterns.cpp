#include "overlapping_patterns.h"
#include "directions.h"
#include "hash_boost.h"
#include "sprite_holder.h"
#include "sprite_transforms.h"
#include "wfc_globals.h"
#include "wfc_settings.h"
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

OverlappingPatterns::OverlappingPatterns(const SpriteHolder &sprite, int N,
                                         BoundaryCondition boundary_condition,
                                         uint8_t transform_flags) {
  this->N = N;
  channels = sprite.getChannels();

  std::vector<SpriteHolder> transformed_sprites = {};
  if (transform_flags == 0) {
    transform_flags = SpriteTransforms::IDENTITY;
  }
  while (transform_flags) {
    uint8_t current_transform = transform_flags & ~(transform_flags - 1);
    transform_flags &= ~current_transform;
    transformed_sprites.push_back(sprite.computeTransform(current_transform));
  }

  computeGridSize(transformed_sprites, boundary_condition);
  computePatternIds(transformed_sprites, boundary_condition);
  mapIdsToPixels(transformed_sprites);
  countPatterns();
  findBoundaryPatterns();
  generateAdjacencyData();
}

void OverlappingPatterns::computeGridSize(const std::vector<SpriteHolder> &transformed_sprites,
                                          BoundaryCondition boundary_condition) {
  total_transforms = transformed_sprites.size();

  size_t width = transformed_sprites[0].getWidth() - N + 1;
  size_t height = transformed_sprites[0].getHeight() - N + 1;
  if (boundary_condition == BoundaryCondition::PERIODIC_X ||
      boundary_condition == BoundaryCondition::PERIODIC) {
    width = transformed_sprites[0].getWidth() + 1;
  }
  if (boundary_condition == BoundaryCondition::PERIODIC_Y ||
      boundary_condition == BoundaryCondition::PERIODIC) {
    height = transformed_sprites[0].getHeight() + 1;
  }
  grid_size = width * height;
}

/* Give unique identifiers to each pattern found in the input data*/
void OverlappingPatterns::computePatternIds(const std::vector<SpriteHolder> &transformed_sprites,
                                            BoundaryCondition boundary_condition) {

  widths.clear();
  heights.clear();

  ids_to_pixels.clear();
  grid_pattern_ids.clear();
  grid_pattern_ids.reserve(grid_size * total_transforms);
  std::unordered_map<pattern_hash_t, pattern_id_t> hashes_to_ids = {};
  for (size_t sprite_index = 0; sprite_index < total_transforms; ++sprite_index) {
    const SpriteHolder &sprite = transformed_sprites[sprite_index];
    size_t width = sprite.getWidth() - N + 1;
    size_t height = sprite.getHeight() - N + 1;

    if (boundary_condition == BoundaryCondition::PERIODIC_X ||
        boundary_condition == BoundaryCondition::PERIODIC) {
      width = sprite.getWidth() + 1;
    }
    if (boundary_condition == BoundaryCondition::PERIODIC_Y ||
        boundary_condition == BoundaryCondition::PERIODIC) {
      height = sprite.getHeight() + 1;
    }
    widths.push_back(width);
    heights.push_back(height);

    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        pattern_hash_t pattern_hash = 0;
        for (size_t dy = 0; dy < N; dy++) {
          size_t adj_y = (y + dy) % sprite.getHeight();
          for (size_t dx = 0; dx < N; dx++) {
            size_t adj_x = (x + dx) % sprite.getWidth();
            const pixel_hash_t pixelHash = sprite.getPixelHash(adj_x, adj_y);
            pattern_hash = hash_combine(pattern_hash, pixelHash);
          }
        }
        if (!hashes_to_ids.contains(pattern_hash)) {
          hashes_to_ids[pattern_hash] = static_cast<pattern_id_t>(hashes_to_ids.size());
          // for (size_t dy = 0; dy < N; ++dy) {
          //   size_t adj_x = x % sprite.getWidth();
          //   size_t adj_y = (y + dy) % sprite.getHeight();
          //   const uint8_t *pixel_row = sprite.getPixelPointer(adj_x, adj_y);
          //   ids_to_pixels.insert(ids_to_pixels.end(), pixel_row, pixel_row + N * channels);
          // }
        }
        grid_pattern_ids[y * width + x + sprite_index * grid_size] = hashes_to_ids[pattern_hash];
      }
    }
  }
  num_patterns = hashes_to_ids.size();
}

void OverlappingPatterns::mapIdsToPixels(const std::vector<SpriteHolder> &transformed_sprites) {
  size_t pattern_id = 0;
  size_t pattern_size = N * N * channels;
  ids_to_pixels.clear();
  ids_to_pixels.resize(num_patterns * pattern_size);

  for (size_t sprite_index = 0; sprite_index < transformed_sprites.size(); ++sprite_index) {
    const SpriteHolder &sprite = transformed_sprites[sprite_index];
    size_t width = widths[sprite_index];
    size_t height = heights[sprite_index];
    size_t bytes_per_row = N * channels * sizeof(uint8_t);

    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        size_t adj_x = x % sprite.getWidth();
        if (grid_pattern_ids[y * width + x + sprite_index * grid_size] == pattern_id) {
          size_t start_index = pattern_id * pattern_size;
          for (size_t dy = 0; dy < N; ++dy) {
            size_t adj_y = (y + dy) % sprite.getHeight();
            const uint8_t *pixel_row = sprite.getPixelPointer(adj_x, adj_y);
            uint8_t *pixel_destination = &ids_to_pixels[start_index + dy * N * channels];
            std::copy(pixel_row, pixel_row + N * channels, pixel_destination);
            // std::memcpy(&ids_to_pixels[start_index + dy * N * channels], pixel_row,
            // bytes_per_row);
          }
          ++pattern_id;
        }
      }
    }
  }
}

void OverlappingPatterns::countPatterns() {
  pattern_frequencies.clear();
  pattern_frequencies.resize(num_patterns, 0);
  for (pattern_id_t pattern_id : grid_pattern_ids) {
    ++pattern_frequencies[pattern_id];
  }
}

void OverlappingPatterns::findBoundaryPatterns() {
  size_t num64_blocks = (num_patterns + 63) / 64;
  patterns_at_boundaries.clear();
  patterns_at_boundaries.resize(NUM_DIRECTIONS_2D * num64_blocks, 0);

  for (size_t sprite_index = 0; sprite_index < total_transforms; ++sprite_index) {
    size_t width = widths[sprite_index];
    size_t height = heights[sprite_index];

    // TOP and BOTTOM ROWS
    std::vector<size_t> y_indexes = {0, height - 1};
    std::vector<size_t> start_indexes = {Directions::UP * num64_blocks,
                                         Directions::DOWN * num64_blocks};
    for (size_t i = 0; i < 2; ++i) {
      size_t y = y_indexes[i];
      size_t start_index = start_indexes[i];
      for (size_t x = 0; x < width; x++) {
        pattern_id_t pattern_id = grid_pattern_ids[y * width + x + sprite_index * grid_size];
        size_t block_index = pattern_id / 64;
        size_t bit_index = pattern_id % 64;
        patterns_at_boundaries[start_index + block_index] |= (1ULL << bit_index);
      }
    }

    // LEFT and RIGHT COLUMNs
    std::vector<size_t> x_indexes = {0, width - 1};
    start_indexes = {Directions::LEFT * num64_blocks, Directions::RIGHT * num64_blocks};
    for (size_t i = 0; i < 2; ++i) {
      size_t x = x_indexes[i];
      size_t start_index = start_indexes[i];
      for (size_t y = 1; y < height - 1; ++y) {
        pattern_id_t pattern_id = grid_pattern_ids[y * width + x + sprite_index * grid_size];
        size_t block_index = pattern_id / 64;
        size_t bit_index = pattern_id % 64;
        patterns_at_boundaries[start_index + block_index] |= (1ULL << bit_index);
      }
    }
  }
}

std::vector<uint8_t>
OverlappingPatterns::convertIdsToPixels(std::span<const pattern_id_t> pattern_ids, size_t width,
                                        size_t height) const {
  std::vector<uint8_t> output_pixels;
  size_t output_width = width + N - 1;
  size_t output_height = height + N - 1;
  size_t bytes_per_row = N * channels * sizeof(uint8_t);
  output_pixels.resize(output_width * output_height * channels);
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      pattern_id_t pattern_id = pattern_ids[y * width + x];
      const uint8_t *pixel_data = &ids_to_pixels[pattern_id * channels * N * N];
      for (size_t dy = 0; dy < N; dy++) {
        size_t pixel_index = dy * N * channels;
        std::memcpy(&output_pixels[((y + dy) * output_width + x) * channels],
                    &pixel_data[pixel_index], bytes_per_row);
      }
    }
  }
  return output_pixels;
}

std::vector<uint8_t> OverlappingPatterns::getInputPixelPatterns() const {
  size_t pattern_size = N * N * channels;

  std::vector<uint8_t> output_pixels;
  output_pixels.resize(num_patterns * pattern_size);
  for (size_t i = 0; i < num_patterns; i++) {
    const uint8_t *pixel_data = &ids_to_pixels[i * pattern_size];
    std::memcpy(&output_pixels[i * pattern_size], pixel_data, pattern_size * sizeof(uint8_t));
  }
  return output_pixels;
}

AdjacencyData OverlappingPatterns::generateAdjacencyData() const {
  std::vector<std::unordered_map<pattern_id_t, uint64_t>> adjacent_patterns(num_patterns *
                                                                            NUM_DIRECTIONS_2D);

  for (size_t sprite_index = 0; sprite_index < total_transforms; ++sprite_index) {
    size_t width = widths[sprite_index];
    size_t height = heights[sprite_index];
    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        if (x > 0) {
          pattern_id_t left_id = grid_pattern_ids[y * width + (x - 1) + sprite_index * grid_size];
          pattern_id_t right_id = grid_pattern_ids[y * width + x + sprite_index * grid_size];
          ++adjacent_patterns[left_id * NUM_DIRECTIONS_2D + Directions::RIGHT][right_id];
          ++adjacent_patterns[right_id * NUM_DIRECTIONS_2D + Directions::LEFT][left_id];
        }
        if (y < height - 1) {
          // Image reads from top to bottom, so the pattern below is at y+1
          size_t bottom_id = grid_pattern_ids[(y + 1) * width + x + sprite_index * grid_size];
          size_t top_id = grid_pattern_ids[y * width + x + sprite_index * grid_size];
          ++adjacent_patterns[bottom_id * NUM_DIRECTIONS_2D + Directions::DOWN][top_id];
          ++adjacent_patterns[top_id * NUM_DIRECTIONS_2D + Directions::UP][bottom_id];
        }
      }
    }
  }

  return AdjacencyData(adjacent_patterns, pattern_frequencies, patterns_at_boundaries);
}