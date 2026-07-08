#include <sprite_holder.h>
#include <stdexcept>

/* Create a unique hash for each pixel in the image.*/
// WARNING: this assumes RBG format, it will not work otherwise
void SpriteHolder::computePixelHashes() {
  pixel_hashes.resize(num_pixels);
  for (int i = 0; i < num_pixels; i++) {
    pixel_hashes[i] = image_pixels[i * channels] |
                      (image_pixels[i * channels + 1] << 8) |
                      (image_pixels[i * channels + 2] << 16);
  }
}

uint32_t SpriteHolder::getPixelHash(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) {
    throw std::out_of_range("Pixel coordinates out of bounds");
  }
  return pixel_hashes[y * width + x];
}