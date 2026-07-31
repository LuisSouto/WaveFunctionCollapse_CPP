#include "wfc_globals.h"
#include <directions.h>
#include <hash_boost.h>
#include <image_transforms.h>
#include <overlapping_patterns.h>
#include <sprite_holder.h>
#include <bit>
#include <unordered_map>
#include <vector>

OverlappingPatterns::OverlappingPatterns(const SpriteHolder &sprite, int N,
		BoundaryCondition boundary_condition, uint8_t transforms) {
	if (transforms == 0) {
		transforms = ImageTransform::IDENTITY;
	}

	transform_indexes.clear();
	// The positions of the bits are the indexes of the transforms to be applied
	while (transforms) {
		uint8_t transform_index = std::countr_zero(transforms);
		transform_indexes.push_back(transform_index);
		transforms &= transforms - 1;
	}

	computePatternHashes(sprite, N, boundary_condition);
	mapHashesToIds();
	computeGridIds();
	mapIdsToPixels(sprite);
	countPatterns();
	findBoundaryPatterns();
	generateAdjacentData();
}

void OverlappingPatterns::computePatternHashes(
		const SpriteHolder &sprite, size_t N, BoundaryCondition boundary_condition) {
	this->N = N;

	sprite_width = sprite.getWidth();
	sprite_height = sprite.getHeight();
	width = sprite.getWidth() - N + 1;
	height = sprite.getHeight() - N + 1;
	grid_size = width * height;

	if (boundary_condition == BoundaryCondition::PERIODIC_X ||
			boundary_condition == BoundaryCondition::PERIODIC) {
		width = sprite.getWidth() + 1;
	}
	if (boundary_condition == BoundaryCondition::PERIODIC_Y ||
			boundary_condition == BoundaryCondition::PERIODIC) {
		height = sprite.getHeight() + 1;
	}

	channels = sprite.getChannels();
	grid_pattern_hashes.clear();
	grid_pattern_hashes.resize(grid_size * transform_indexes.size());
	for (size_t t = 0; t < transform_indexes.size(); t++) {
		TransformedIndex transform_func = all_transforms[transform_indexes[t]];
		for (size_t y = 0; y < height; y++) {
			for (size_t x = 0; x < width; x++) {
				pattern_hash_t pattern_hash = 0;
				for (size_t dy = 0; dy < N; dy++) {
					size_t adj_y = (y + dy) % sprite_height;
					for (size_t dx = 0; dx < N; dx++) {
						size_t adj_x = (x + dx) % sprite_width;
						size_t transformed_index =
								transform_func(adj_x, adj_y, sprite_width, sprite_height);
						const pixel_hash_t pixelHash = sprite.getPixelHash(transformed_index);
						pattern_hash = hash_combine(pattern_hash, pixelHash);
					}
				}
				grid_pattern_hashes[transform_func(x, y, width, height) + grid_size * t] =
						pattern_hash;
			}
		}
	}
}

void OverlappingPatterns::mapHashesToIds() {
	for (pattern_hash_t pattern_hash : grid_pattern_hashes) {
		if (!hashes_to_ids.contains(pattern_hash)) {
			hashes_to_ids[pattern_hash] = static_cast<pattern_id_t>(hashes_to_ids.size());
		}
	}
}

void OverlappingPatterns::computeGridIds() {
	grid_pattern_ids.clear();
	grid_pattern_ids.reserve(grid_pattern_hashes.size());
	for (pattern_hash_t pattern_hash : grid_pattern_hashes) {
		grid_pattern_ids.push_back(hashes_to_ids[pattern_hash]);
	}
}

void OverlappingPatterns::mapIdsToPixels(const SpriteHolder &sprite) {
	ids_to_pixels.clear();
	ids_to_pixels.resize(hashes_to_ids.size() * N * N * channels);
	size_t pattern_id = 0;
	for (size_t t = 0; t < transform_indexes.size(); ++t) {
		TransformedIndex transform_func = all_transforms[transform_indexes[t]];
		for (size_t y = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x) {
				size_t adj_x = x % sprite.getWidth();
				pattern_hash_t pattern_hash =
						grid_pattern_hashes[transform_func(x, y, width, height) + grid_size * t];
				if (hashes_to_ids[pattern_hash] == pattern_id) {
					size_t start_index = pattern_id * N * N * channels;
					for (size_t dy = 0; dy < N; ++dy) {
						size_t adj_y = (y + dy) % sprite.getHeight();
						const uint8_t *pixel_row = sprite.getPixelPointer(transform_func(
								adj_x, adj_y, sprite.getWidth(), sprite.getHeight()));
						uint8_t *pixel_destination =
								&ids_to_pixels[start_index + dy * N * channels];
						std::copy(pixel_row, pixel_row + N * channels, pixel_destination);
					}
					++pattern_id;
				}
			}
		}
	}
}

void OverlappingPatterns::countPatterns() {
	pattern_frequencies.clear();
	pattern_frequencies.resize(hashes_to_ids.size(), 0);
	for (pattern_id_t pattern_id : grid_pattern_ids) {
		++pattern_frequencies[pattern_id];
	}
}

void OverlappingPatterns::findBoundaryPatterns() {
	size_t num64_blocks = (hashes_to_ids.size() + 63) / 64;
	patterns_at_boundaries.clear();
	patterns_at_boundaries.resize(NUM_DIRECTIONS_2D * num64_blocks, 0);

	for (size_t t = 0; t < transform_indexes.size(); ++t) {
		size_t transform_index = transform_indexes[t];
		TransformedIndex transform_func = all_transforms[transform_index];

		// TOP and BOTTOM ROWS
		std::vector<size_t> y_indexes = { 0, height - 1 };
		std::vector<size_t> start_indexes = {
			rotateDirectionClockwise(Directions::UP, transform_index) * num64_blocks,
			rotateDirectionClockwise(Directions::DOWN, transform_index) * num64_blocks
		};
		for (size_t i = 0; i < 2; ++i) {
			size_t y = y_indexes[i];
			size_t start_index = start_indexes[i];
			for (size_t x = 0; x < width; x++) {
				pattern_id_t pattern_id =
						grid_pattern_ids[transform_func(x, y, width, height) + grid_size * t];
				size_t block_index = pattern_id / 64;
				size_t bit_index = pattern_id % 64;
				patterns_at_boundaries[start_index + block_index] |= (1ULL << bit_index);
			}
		}

		// LEFT and RIGHT COLUMNs
		std::vector<size_t> x_indexes = { 0, width - 1 };
		start_indexes = { rotateDirectionClockwise(Directions::LEFT, transform_index) *
					num64_blocks,
			rotateDirectionClockwise(Directions::RIGHT, transform_index) * num64_blocks };
		for (size_t i = 0; i < 2; ++i) {
			size_t x = x_indexes[i];
			size_t start_index = start_indexes[i];
			for (size_t y = 1; y < height - 1; ++y) {
				pattern_id_t pattern_id =
						grid_pattern_ids[transform_func(x, y, width, height) + grid_size * t];
				size_t block_index = pattern_id / 64;
				size_t bit_index = pattern_id % 64;
				patterns_at_boundaries[start_index + block_index] |= (1ULL << bit_index);
			}
		}
	}
}

std::vector<uint8_t> OverlappingPatterns::convertIdsToPixels(
		std::span<const pattern_id_t> pattern_ids, size_t width, size_t height) const {
	std::vector<uint8_t> output_pixels;
	size_t output_width = width + N - 1;
	size_t output_height = height + N - 1;
	output_pixels.resize(output_width * output_height * channels);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			pattern_id_t pattern_id = pattern_ids[y * width + x];
			const uint8_t *pixel_data = &ids_to_pixels[pattern_id * channels * N * N];
			for (size_t dy = 0; dy < N; dy++) {
				size_t pixel_index = dy * N * channels;
				std::memcpy(&output_pixels[((y + dy) * output_width + x) * channels],
						&pixel_data[pixel_index], N * channels);
			}
		}
	}
	return output_pixels;
}

std::vector<uint8_t> OverlappingPatterns::getInputPixelPatterns() const {
	size_t num_patterns = hashes_to_ids.size();
	size_t pattern_size = N * N * channels;

	std::vector<uint8_t> output_pixels;
	output_pixels.resize(num_patterns * pattern_size);
	for (size_t i = 0; i < num_patterns; i++) {
		const uint8_t *pixel_data = &ids_to_pixels[i * pattern_size];
		std::memcpy(&output_pixels[i * pattern_size], pixel_data, pattern_size);
	}
	return output_pixels;
}

// TODO: for now we do not use rotations, mirroring, wrapping, or any other
// pattern augmentation
AdjacencyData OverlappingPatterns::generateAdjacentData() const {
	// DISCOVERY PHASE: look for adjacent patterns in the grid and count their
	// frequencies
	std::vector<std::unordered_map<pattern_id_t, uint64_t>> adjacent_patterns(
			hashes_to_ids.size() * NUM_DIRECTIONS_2D);
	for (size_t t = 0; t < transform_indexes.size(); ++t) {
		size_t transform_index = transform_indexes[t];
		TransformedIndex transform_func = all_transforms[transform_index];
		for (size_t y = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x) {
				if (x > 0) {
					pattern_id_t left_id =
							grid_pattern_ids[transform_func(x - 1, y, width, height) +
									grid_size * t];
					pattern_id_t right_id =
							grid_pattern_ids[transform_func(x, y, width, height) + grid_size * t];
					++adjacent_patterns[left_id * NUM_DIRECTIONS_2D +
							rotateDirectionClockwise(Directions::RIGHT, transform_index)][right_id];
					++adjacent_patterns[right_id * NUM_DIRECTIONS_2D +
							rotateDirectionClockwise(Directions::LEFT, transform_index)][left_id];
				}
				if (y < height - 1) {
					// Image reads from top to bottom, so the pattern below is at y+1
					size_t bottom_id = grid_pattern_ids[transform_func(x, y + 1, width, height) +
							grid_size * t];
					size_t top_id =
							grid_pattern_ids[transform_func(x, y, width, height) + grid_size * t];
					++adjacent_patterns[bottom_id * NUM_DIRECTIONS_2D +
							rotateDirectionClockwise(Directions::DOWN, transform_index)][top_id];
					++adjacent_patterns[top_id * NUM_DIRECTIONS_2D +
							rotateDirectionClockwise(Directions::UP, transform_index)][bottom_id];
				}
			}
		}
	}

	return AdjacencyData(adjacent_patterns, pattern_frequencies, patterns_at_boundaries);
}