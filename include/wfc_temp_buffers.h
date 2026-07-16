#pragma once

#include <cstdint>
#include <vector>
#include <wfc_typedefs.h>

struct WFCTempBuffers {
  std::vector<pattern_id_t> cell_pattern_ids;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<uint64_t> constraints_from_cell;
  std::vector<uint32_t> cell_update_versions;
  std::vector<size_t> current_cell_wave;
  std::vector<size_t> next_cell_wave;
  uint32_t current_version = 0;
};