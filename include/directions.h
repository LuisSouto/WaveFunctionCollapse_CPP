#pragma once

#include <vector>

enum Directions { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

const static std::vector<int> DX = {0, 1, 0, -1};
const static std::vector<int> DY = {1, 0, -1, 0};

static size_t rotateDirectionClockwise(Directions direction, size_t num_rotations) {
  return direction;
}