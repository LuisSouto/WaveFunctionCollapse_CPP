#include <sprite_reader.h>
#include <stb_image.h>
#include <cstddef>
#include <cstdint>
#include <vector>

SpriteHolder SpriteReader::loadFromPng(const char *filename, int desired_channels) {
	int width;
	int height;
	int n_channels;

	unsigned char *image_data = stbi_load(filename, &width, &height, &n_channels, desired_channels);

	assert(image_data && "Failed to load image");

	// Make into flat array for cache locality
	size_t total_bytes = static_cast<size_t>(width) * height * desired_channels * sizeof(uint8_t);
	std::vector<uint8_t> image_pixels_rgb(image_data, image_data + total_bytes);
	stbi_image_free(image_data);

	return SpriteHolder(static_cast<size_t>(width), static_cast<size_t>(height), desired_channels,
			image_pixels_rgb);
}