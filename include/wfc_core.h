#pragma once

#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <vector>
#include <wfc_settings.h>
#include <wfc_temp_buffers.h>
#include <wfc_typedefs.h>

class WFC {
private:
  // TODO: group variables in the way that they are used together to improve
  // cache locality
  uint64_t num_collapsed_cells = 0;
  uint64_t snapshot_num_collapsed_cells = 0;
  size_t current_block_x = 0;
  size_t current_block_y = 0;
  AdjacencyData adjacent_data;
  size_t output_width;
  size_t output_height;
  size_t total_cells;
  std::vector<uint64_t> grid;
  std::vector<size_t> neighbour_indexes;
  std::vector<uint64_t> backtracking_block;
  std::vector<uint8_t> is_outside_block;
  std::vector<uint8_t> is_cell_collapsed;
  std::vector<pattern_id_t> collapsed_patterns;
  std::vector<pattern_id_t> cell_pattern_ids;
  std::vector<uint64_t> pattern_frequencies;
  WFCTempBuffers temp_buffers;
  WFCSettings settings;
  std::mt19937_64 rng; // Random number generator for selecting patterns
  void initializeGrid(size_t output_width, size_t output_height);
  void bakeNeighbourIndexes();
  size_t findCellToCollapse(size_t previous_cell_index);
  std::span<const pattern_id_t> readPatternsAtCell(size_t cell_index);
  void collapsePatternAtCell(size_t cell_index);
  uint8_t propagateConstraints(size_t cell_index,
                               uint8_t is_global_propagation);
  uint8_t extendPropagationRange(uint8_t global_propagation);
  std::pair<uint8_t, uint8_t>
  updateConstraintsOfNeighbour(std::span<const uint64_t> cell_constraints,
                               size_t neighbour_index);
  void updateBackTrackingBlock();
  std::span<const uint64_t> getConstraintsFromCell(size_t cell_index);
  size_t restartBlock();
  size_t moveToNextBlock();

public:
  WFC(const AdjacencyData &adjacent_data) : adjacent_data(adjacent_data) {
    std::random_device rd;
    rng = std::mt19937_64(rd());
  };
  WFC(const AdjacencyData &adjacent_data, uint64_t seed)
      : adjacent_data(adjacent_data), rng(seed) {};

  std::span<const pattern_id_t> generateCollapsedGrid(size_t output_width,
                                                      size_t output_height);
};