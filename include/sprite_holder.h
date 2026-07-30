#pragma once

#include <wfc_typedefs.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

class SpriteHolder {
private:
	size_t width;
	size_t height;
	size_t channels;
	size_t num_pixels;
	std::vector<uint8_t> image_pixels;
	std::vector<pixel_hash_t> pixel_hashes;
	void computePixelHashes();

public:
	SpriteHolder(size_t width, size_t height, size_t channels, std::vector<uint8_t> image_pixels) :
			width(width),
			height(height),
			channels(channels),
			image_pixels(std::move(image_pixels)) {
		num_pixels = width * height;
		computePixelHashes();
	};

	size_t getWidth() const { return width; }
	size_t getHeight() const { return height; }
	size_t getChannels() const { return channels; }
	const std::vector<uint8_t> &getImagePixels() const { return image_pixels; }
	const std::vector<pixel_hash_t> &getPixelHashes() const { return pixel_hashes; }
	pixel_hash_t getPixelHash(size_t x, size_t y) const {
		assert(x < width && y < height && "Pixel coordinates out of bounds");
		return pixel_hashes[y * width + x];
	};
	pixel_hash_t getPixelHash(size_t cell_index) const {
		assert(cell_index < num_pixels && "Pixel coordinates out of bounds");
		return pixel_hashes[cell_index];
	};

	const uint8_t *getPixelPointer(size_t x, size_t y) const {
		assert(x < width && y < height && "Pixel coordinates out of bounds");
		return &image_pixels[(y * width + x) * channels];
	};
};