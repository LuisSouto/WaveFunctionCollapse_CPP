#pragma once

#include "adjacency_data.h"
#include "sprite_holder.h"
#include "wfc_settings.h"
#include "wfc_typedefs.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

/*Looks for patterns in a sample image using overlapping.*/
class OverlappingPatterns {
private:
  std::vector<SpriteHolder> transformed_sprites;
  std::vector<size_t> widths;
  std::vector<size_t> heights;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<uint64_t> patterns_at_boundaries;
  std::vector<pattern_id_t> grid_pattern_ids;
  std::vector<uint8_t> ids_to_pixels;
  size_t N;
  size_t channels;
  size_t grid_size;
  size_t total_transforms;
  size_t num_patterns;
  void computeGridSize(const std::vector<SpriteHolder> &transformed_sprites,
                       BoundaryCondition boundary_condition);
  void computePatternIds(const std::vector<SpriteHolder> &transformed_sprites,
                         BoundaryCondition boundary_condition);
  void findBoundaryPatterns();
  void mapIdsToPixels(const std::vector<SpriteHolder> &transformed_sprites);
  void countPatterns();

public:
  OverlappingPatterns(const SpriteHolder &sprite, int N, BoundaryCondition boundary_condition,
                      uint8_t transform_flags);

  AdjacencyData generateAdjacencyData() const;

  size_t getNumPatterns() const { return num_patterns; }

  std::vector<uint8_t> convertIdsToPixels(std::span<const pattern_id_t> pattern_ids, size_t width,
                                          size_t height) const;

  std::vector<uint8_t> getInputPixelPatterns() const;
};
