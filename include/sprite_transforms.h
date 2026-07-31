#pragma once

#include <vector>

struct SpriteTransforms {
	enum TransformType {
		IDENTITY = 1,
		ROTATE_90 = 2,
		ROTATE_180 = 4,
		ROTATE_270 = 8,
		ALL_TRANSFORMS = 15
	};
};

typedef size_t (*TransformFunction)(size_t x, size_t y, size_t width, size_t height);

inline size_t identity(size_t x, size_t y, size_t width, size_t height) { return y * width + x; }

inline size_t rotate90(size_t x, size_t y, size_t width, size_t height) {
	return y + (height - 1 - x) * width;
}

inline size_t rotate180(size_t x, size_t y, size_t width, size_t height) {
	return (width - 1 - x) + (height - 1 - y) * width;
}

inline size_t rotate270(size_t x, size_t y, size_t width, size_t height) {
	return (width - 1 - y) + x * width;
}

inline std::vector<TransformFunction> transformFunctions = { identity, rotate90, rotate180,
	rotate270 };