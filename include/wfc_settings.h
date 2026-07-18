#pragma once

#include <cstdint>

enum BoundaryCondition { NONE, PERIODIC_X, PERIODIC_Y, PERIODIC, FIXED };

enum CellSelectionStrategy { SCANLINE, ENTROPY };

struct WFCSettings {
  uint8_t dimensions = 2;
  uint8_t boundary_condition = BoundaryCondition::PERIODIC_X;
  uint8_t cell_selection_strategy = CellSelectionStrategy::SCANLINE;
  uint32_t block_size_x = 16;
  uint32_t block_size_y = 16;
  uint8_t enable_backtracking = false;
  uint8_t force_boundary_patterns = true;
};