#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include <wfc_globals.h>
#include <wfc_typedefs.h>

class AdjacencyData {
private:
  size_t num_patterns;
  size_t num_64_blocks;
  std::vector<uint64_t> neighbour_ids;
  std::vector<uint64_t> pattern_frequencies;
  // For each pattern and direction, store the offsets of its adjacent patterns
  // in the grid
  std::vector<size_t> neighbour_offsets;
  // For each pattern and direction, store the count of its adjacent patterns
  std::vector<size_t> neighbour_counts;

public:
  AdjacencyData() = default;
  AdjacencyData(
      std::vector<std::unordered_map<pattern_id_t, uint64_t>> &discovered_maps,
      size_t width, size_t height);
  size_t getNumPatterns() const { return num_patterns; }
  size_t getNum64Blocks() const { return num_64_blocks; }
  uint64_t getPatternFrequency(pattern_id_t pattern_id) {
    return pattern_frequencies[pattern_id];
  }
  std::span<const uint64_t> getNeighbourIds(size_t cell_index,
                                            uint8_t direction) const {

    size_t offset =
        (cell_index * NUM_DIRECTIONS_2D + direction) * num_64_blocks;
    return {&neighbour_ids[offset], num_64_blocks};
  }
};