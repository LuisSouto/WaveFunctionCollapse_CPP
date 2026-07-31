#pragma once

#include <vector>

enum Directions { LEFT = 0, RIGHT = 1, DOWN = 2, UP = 3 };

const static std::vector<int> DX = {-1, 1, 0, 0};
const static std::vector<int> DY = {0, 0, -1, 1};