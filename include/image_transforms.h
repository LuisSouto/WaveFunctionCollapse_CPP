#pragma once

#include <vector>

struct ImageTransform {
  enum { IDENTITY = 1, ROTATE_90 = 2, ROTATE_180 = 4, ROTATE_270 = 8 };
};

typedef size_t (*TransformedIndex)(size_t x, size_t y, size_t width, size_t height);

size_t identityIndex(size_t x, size_t y, size_t width, size_t height) { return y * width + x; }

size_t rotated90Index(size_t x, size_t y, size_t width, size_t height) {
  return (height - 1 - x) * width + y;
}

size_t rotated180Index(size_t x, size_t y, size_t width, size_t height) {
  return (height - 1 - y) * width + (width - 1 - x);
}

size_t rotated270Index(size_t x, size_t y, size_t width, size_t height) {
  return (width - 1 - y) * height + x;
}

const std::vector<TransformedIndex> all_transforms = {identityIndex, rotated90Index,
                                                      rotated180Index, rotated270Index};