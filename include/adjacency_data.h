#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
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
};