#ifndef SPRITE_HOLDER_H
#define SPRITE_HOLDER_H

#include <cstdint>
#include <vector>

class SpriteHolder {

private:
  int width;
  int height;
  int channels;
  int num_pixels;
  std::vector<uint8_t> image_pixels;
  std::vector<uint32_t> pixel_hashes;
  void computePixelHashes();

public:
  SpriteHolder(int width, int height, int channels,
               std::vector<uint8_t> image_pixels)
      : width(width), height(height), channels(channels),
        image_pixels(std::move(image_pixels)) {
    num_pixels = width * height;
    computePixelHashes();
  };

  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getChannels() const { return channels; }
  const std::vector<uint8_t> &getImagePixels() const { return image_pixels; }
  const std::vector<uint32_t> &getPixelHashes() const { return pixel_hashes; }
  uint32_t getPixelHash(int x, int y) const;
};

#endif // SPRITE_HOLDER_H