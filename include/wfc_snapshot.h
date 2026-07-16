#pragma once

#include <vector>
#include <wfc_typedefs.h>

struct WFCSnapshot {
  std::vector<uint64_t> grid;
  std::vector<uint8_t> is_cell_collapsed;
  std::vector<pattern_id_t> collapsed_patterns;
  std::vector<double> entropy_weight_times_log_weights;
  std::vector<double> entropy_weight_sums;
  std::vector<uint64_t> grid_block;
  std::vector<uint8_t> is_outside_block;
  uint64_t num_collapsed_cells = 0;
  size_t current_cell_index = 0;
  size_t current_block_x = 0;
  size_t current_block_y = 0;
  uint32_t consecutive_failures = 0;
  uint32_t max_consecutive_failures = 100;
};