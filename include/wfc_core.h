#pragma once

#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <entropy.h>
#include <random>
#include <span>
#include <vector>
#include <wfc_settings.h>
#include <wfc_temp_buffers.h>
#include <wfc_typedefs.h>

class WFC {
private:
  const static uint32_t max_snapshots = 10000;
  std::vector<uint64_t> grid;
  std::vector<uint64_t> undo_stack;
  std::vector<uint64_t> collapsed_mask;
  std::vector<size_t> neighbour_indexes;
  std::vector<size_t> stack_checkpoints;
  std::vector<size_t> stack_starting_cell_indexes;
  std::vector<uint32_t> num_collapsed_cells_at_snapshot;
  std::vector<uint32_t> consecutive_failures_at_snapshot;
  std::vector<pattern_id_t> collapsed_patterns;
  std::vector<uint8_t> is_cell_collapsed;
  WFCTempBuffers temp_buffers;
  AdjacencyData adjacent_data;
  EntropyData entropy_data;
  WFCSettings settings;
  std::mt19937_64 rng;
  std::uniform_real_distribution<> dist{0.0, 1.0};
  uint64_t num_collapsed_cells = 0;
  uint64_t stack_counter = 0;
  size_t output_width;
  size_t output_height;
  size_t start_index;
  size_t total_cells;
  size_t num64_blocks;
  uint32_t failed_snapshots = 0;
  uint32_t max_consecutive_failures = 100;
  uint32_t total_snapshots = 0;
  uint32_t total_failures = 0;
  uint32_t max_total_failures = 5000;
  int scan_direction = 1; // 1 for forward, -1 for backward

  void initializeGrid(size_t output_width, size_t output_height);

  void initializeTempBuffers();

  void initializeEntropyData();

  double calculateEntropyAtCell(size_t cell_index);

  void initializeUndoStack();

  void applyBoundaryConditions();

  void bakeNeighbourIndexes();

  size_t chooseNextCellEntropy();

  size_t chooseNextCellScanline(size_t previous_cell_index);

  std::span<const pattern_id_t> readPatternsAtCell(size_t cell_index);

  void collapsePatternAtCell(size_t cell_index);

  uint8_t propagateConstraints(size_t cell_index);

  uint8_t extendPropagationRange();

  std::pair<uint8_t, uint8_t>
  updateConstraintsOfNeighbour(std::span<const uint64_t> cell_constraints,
                               size_t neighbour_index);

  std::span<const uint64_t> getConstraintsFromCell(size_t cell_index);

  void saveSnapshot(size_t current_cell_index);

  size_t restoreSnapshot();

  size_t backTrackToPreviousSnapshot();

  void pushCellToUndoStack(size_t cell_index);

  bool generateCollapsedGrid(size_t output_width, size_t output_height,
                             size_t start_index);

public:
  WFC(const AdjacencyData &adjacent_data) : adjacent_data(adjacent_data) {
    std::random_device rd;
    rng = std::mt19937_64(rd());
  };

  WFC(const AdjacencyData &adjacent_data, uint64_t seed)
      : adjacent_data(adjacent_data), rng(seed) {};

  std::span<const pattern_id_t> solve(size_t output_width, size_t output_height,
                                      size_t start_index);
};