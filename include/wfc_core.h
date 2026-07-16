#pragma once

#include <adjacency_data.h>
#include <cstddef>
#include <cstdint>
#include <entropy.h>
#include <random>
#include <span>
#include <vector>
#include <wfc_settings.h>
#include <wfc_snapshot.h>
#include <wfc_temp_buffers.h>
#include <wfc_typedefs.h>

class WFC {
private:
  const static uint32_t max_snapshots = 100;
  std::vector<uint64_t> grid;
  std::vector<size_t> neighbour_indexes;
  std::vector<uint8_t> is_cell_collapsed;
  std::vector<pattern_id_t> collapsed_patterns;
  WFCTempBuffers temp_buffers;
  AdjacencyData adjacent_data;
  WFCSnapshot snapshot[max_snapshots];
  EntropyData entropy_data;
  WFCSettings settings;
  std::mt19937_64 rng;
  uint64_t num_collapsed_cells = 0;
  uint64_t snapshot_num_collapsed_cells = 0;
  size_t output_width;
  size_t output_height;
  size_t total_cells;
  int total_snapshots = -1;
  uint32_t snapshot_index = 0;
  uint32_t failed_snapshots = 0;

  void initializeGrid(size_t output_width, size_t output_height);

  void initializeEntropyData();

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

  void updateBackTrackingBlock();

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