#include <bit>
#include <cstdint>
#include <cstring>
#include <sprite_holder.h>
#include <sprite_transforms.h>

/* Create a unique hash for each pixel in the image.*/
void SpriteHolder::computePixelHashes() {
  // Note(Luis): Due to the way the hash is computed, this will overflow for images with
  // more than 4 channels. Not really a problem for the intended use.
  pixel_hashes.resize(num_pixels);
  for (size_t i = 0; i < num_pixels; i++) {
    pixel_hash_t pixel_hash = 0;
    for (size_t c = 0; c < channels; c++) {
      pixel_hash += image_pixels[i * channels + c] << (c * 8);
    }
    pixel_hashes[i] = pixel_hash;
  }
}

/* Create a transformed (i.e. rotated or mirrored) version of the original sprite */
SpriteHolder SpriteHolder::computeTransform(uint8_t transform_flag) const {
  // Kinda superfluous but basically make sure at least the identity is present
  if (transform_flag == 0) {
    transform_flag = SpriteTransforms::IDENTITY;
  }

  if (transform_flag == SpriteTransforms::IDENTITY) {
    return *this;
  }

  assert(std::popcount(transform_flag) == 1 && "Only one transformation can be applied at a time.");

  assert(transform_flag < SpriteTransforms::ALL_TRANSFORMS && "Invalid transformation flag.");

  size_t new_width = width;
  size_t new_height = height;
  if (transform_flag == SpriteTransforms::ROTATE_90 ||
      transform_flag == SpriteTransforms::ROTATE_270) {
    new_width = height;
    new_height = width;
  }
  // uint8_t is 1 byte anyway but just for clarity, I may forget in the future
  size_t bytes_per_pixel = channels * sizeof(uint8_t);
  size_t transform_index = std::countr_zero(transform_flag);
  std::vector<uint8_t> transformed_pixels(num_pixels * channels);
  TransformFunction transform_map = transformMappings[transform_index];
  for (size_t y = 0; y < new_height; ++y) {
    for (size_t x = 0; x < new_width; ++x) {
      // Pixel coordinate in the new image
      size_t pixel_index = y * new_width + x;
      // Corresponding pixel coordinate in the original image
      size_t original_index = transform_map(x, y, width, height);
      std::memcpy(&transformed_pixels[pixel_index * channels],
                  &image_pixels[original_index * channels], bytes_per_pixel);
    }
  }
  return SpriteHolder(new_width, new_height, channels, transformed_pixels);
}