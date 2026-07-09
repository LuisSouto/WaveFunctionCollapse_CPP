#include <cstddef>
#include <sprite_holder.h>

/* Create a unique hash for each pixel in the image.*/
// WARNING: this assumes RBG format, it will not work otherwise
void SpriteHolder::computePixelHashes() {
  pixel_hashes.resize(num_pixels);
  for (size_t i = 0; i < num_pixels; i++) {
    pixel_hashes[i] = image_pixels[i * channels] |
                      (image_pixels[i * channels + 1] << 8) |
                      (image_pixels[i * channels + 2] << 16);
  }
}