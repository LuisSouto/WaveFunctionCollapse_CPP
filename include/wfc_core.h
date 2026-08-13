#pragma once

#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <entropy.h>
#include <random>
#include <span>
#include <unordered_map>
#include <vector>
#include <wfc_settings.h>
#include <wfc_temp_buffers.h>
#include <wfc_typedefs.h>

class WFCCore {
private:
  std::vector<uint64_t> grid;
  std::vector<uint64_t> undo_stack;
  std::vector<uint64_t> collapsed_mask;
  std::vector<size_t> neighbour_indexes;
  std::vector<size_t> stack_checkpoints;
  std::vector<size_t> stack_starting_cell_indexes;
  std::vector<uint32_t> num_collapsed_cells_at_snapshot;
  std::vector<uint32_t> failures_at_snapshots;
  std::vector<pattern_id_t> collapsed_patterns;
  std::vector<uint8_t> is_cell_collapsed;
  WFCTempBuffers temp_buffers;
  AdjacencyData adjacent_data;
  EntropyData entropy_data;
  WFCSettings settings;
  std::mt19937_64 rng;
  std::uniform_real_distribution<> dist{0.0, 1.0};
  size_t grid_width;
  size_t grid_height;
  size_t start_index;
  size_t total_cells;
  size_t num64_blocks;
  uint64_t num_collapsed_cells = 0;
  uint64_t stack_counter = 0;
  uint32_t failed_snapshots = 0;
  uint32_t max_failed_snapshots = 100;
  uint32_t current_snapshot = 0;
  uint32_t max_failures_per_snapshot = 20;
  uint32_t num_contradictions = 0;
  uint32_t max_contradictions = 1;
  uint32_t max_restarts = 500;
  uint32_t stack_size;
  int scan_direction = 1; // 1 for forward, -1 for backward
  CellSelectionStrategy selection_strategy;

  void initializeGrid(size_t output_width, size_t output_height);

  void collapsedFixedCells(const std::unordered_map<size_t, pattern_id_t> &fixed_cells);

  void initializeTempBuffers();

  void initializeEntropyData();

  double calculateEntropyAtCell(size_t cell_index);

  void initializeUndoStack();

  void applyBoundaryConditions();

  void bakeNeighbourIndexes();

  size_t chooseNextCellEntropy();

  size_t chooseNextCellScanline(size_t previous_cell_index);

  std::span<const pattern_id_t> readPatternsAtCell(size_t cell_index);

  void collapsePatternAtCell(size_t cell_index, pattern_id_t pattern_id);

  void collapseRandomPatternAtCell(size_t cell_index);

  uint8_t propagateConstraints(size_t cell_index);

  uint8_t extendPropagationRange();

  std::pair<uint8_t, uint8_t>
  updateConstraintsOfNeighbour(std::span<const uint64_t> cell_constraints, size_t neighbour_index);

  std::span<const uint64_t> getConstraintsFromCell(size_t cell_index);

  void saveSnapshot(size_t current_cell_index);

  size_t restoreSnapshot();

  size_t goToPreviousSnapshot();

  void pushCellToUndoStack(size_t cell_index);

  bool generateCollapsedGrid(int start_index);

public:
  WFCCore(const AdjacencyData &adjacent_data) : adjacent_data(adjacent_data) {
    std::random_device rd;
    rng = std::mt19937_64(rd());
  };

  WFCCore(const AdjacencyData &adjacent_data, uint64_t seed)
      : adjacent_data(adjacent_data), rng(seed) {};

  bool checkIfCellCollapsed(size_t cell_index) const {
    if (cell_index >= total_cells) {
      return false;
    }
    return is_cell_collapsed[cell_index];
  }

  bool collapseSelectedCell(size_t cell_index, pattern_id_t pattern_id);

  int getCollapsedPatternAtCell(size_t cell_index) const {
    if (cell_index >= total_cells) {
      return -1; // if out-of-bounds
    }
    if (!is_cell_collapsed[cell_index]) {
      return -1; // if the cell is not collapsed
    }
    return collapsed_patterns[cell_index];
  }

  std::vector<uint8_t> getValidCellsForPattern(pattern_id_t pattern_id, size_t output_width,
                                               size_t output_height);

  std::span<const pattern_id_t> getCollapsedPatterns() const {
    return {collapsed_patterns.data(), collapsed_patterns.size()};
  }

  void startSolver(size_t grid_width, size_t grid_height, bool force_boundary_patterns,
                   CellSelectionStrategy selection_strategy,
                   const std::unordered_map<size_t, pattern_id_t> &fixed_cells);

  std::span<const pattern_id_t> solve(size_t grid_width, size_t grid_height, int start_index,
                                      bool force_boundary_patterns,
                                      CellSelectionStrategy cell_selection_strategy,
                                      const std::unordered_map<size_t, pattern_id_t> &fixed_cells);

  void undoLastCollapse();
};