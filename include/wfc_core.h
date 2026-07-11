#pragma once

#include "wfc_typedefs.h"
#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

class WFC {
private:
  // TODO: group variables in the way that they are used together to improve
  // cache locality
  uint64_t collapsed_cells = 0;
  std::mt19937_64 rng; // Random number generator for selecting patterns
  AdjacencyData adjacent_data;
  size_t output_width;
  size_t output_height;
  size_t total_cells;
  uint32_t current_version;
  std::vector<uint64_t> grid;
  std::vector<pattern_id_t> cell_possible_pattern_ids;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<pattern_id_t> combined_neighbour_patterns;
  std::vector<uint32_t> cell_update_versions;
  std::vector<size_t> current_cell_wave;
  std::vector<int> direction_offsets;
  void initializeGrid(size_t output_width, size_t output_height);
  size_t findCellToCollapse(size_t previous_cell_index);
  void findPossiblePatternsAtCell(size_t cell_index);
  void collapsePatternForCell(size_t cell_index);
  void propagateConstraints(size_t cell_index);
  void findPossibleNeighbourPatterns(size_t cell_index, uint8_t direction);
  void extendPropagationRange();
  bool isCellWithinBoundaries(size_t cell_index);

public:
  WFC(const AdjacencyData &adjacent_data) : adjacent_data(adjacent_data) {
    std::random_device rd;
    rng = std::mt19937_64(rd());
  };
  WFC(const AdjacencyData &adjacent_data, uint64_t seed)
      : adjacent_data(adjacent_data), rng(seed) {};

  void run(size_t output_width, size_t output_height);
};