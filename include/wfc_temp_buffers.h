#pragma once

#include <cstdint>
#include <vector>

struct WFCTempBuffers {
  std::vector<uint64_t> neighbour_contribution;
  std::vector<uint64_t> constraints_for_neighbour;
  std::vector<uint64_t> constraints_from_cell;
  std::vector<uint32_t> cell_update_versions;
  std::vector<size_t> current_cell_wave;
  std::vector<size_t> next_cell_wave;
  uint32_t current_version = 0;
};