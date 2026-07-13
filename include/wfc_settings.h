#pragma once

#include <cstdint>

enum BoundaryCondition { NONE, PERIODIC, EDGES };

struct WFCSettings {
  uint8_t dimensions = 2;
  uint8_t boundary_condition = BoundaryCondition::NONE;
  uint32_t block_size_x = 16;
  uint32_t block_size_y = 16;
  uint8_t enable_backtracking = false;
};