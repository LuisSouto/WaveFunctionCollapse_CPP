#pragma once

#include <adjacency_data.h>
#include <cstdint>
#include <random>

class WFC {
private:
  std::mt19937_64 rng; // Random number generator for selecting patterns
  AdjacencyData adjacent_data;
  size_t output_width;
  size_t output_height;
  std::vector<uint64_t> grid;
  std::vector<pattern_id_t> possible_pattern_ids;
  std::vector<uint64_t> pattern_frequencies;
  void initializeGrid(size_t output_width, size_t output_height);
  size_t findCellToCollapse(size_t previous_cell_index);
  void findPossiblePatternsAtCell(size_t cell_index);
  void collapsePatternForCell(size_t cell_index);
  void propagateConstraints();

public:
  WFC(const AdjacencyData &adjacent_data) : adjacent_data(adjacent_data) {
    std::random_device rd;
    rng = std::mt19937_64(rd());
  };
  WFC(const AdjacencyData &adjacent_data, uint64_t seed)
      : adjacent_data(adjacent_data), rng(seed) {};

  void run(size_t output_width, size_t output_height);
};