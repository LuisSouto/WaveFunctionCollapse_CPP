#include <iostream>
#include <overlapping_patterns.h>
#include <sprite_reader.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>
#include <wfc_core.h>

int main() {
  std::string filename = "../Sprites/simple_road.png";
  SpriteHolder sprite = SpriteReader::loadFromPng(filename.c_str(), STBI_rgb);
  size_t N = 2;
  OverlappingPatterns overlapping_patterns(sprite, N);
  uint64_t seed = 300;
  WFC wfc(overlapping_patterns.getAdjacencyData(), seed);
  size_t output_width = 128;
  size_t output_height = 128;
  std::span<const pattern_id_t> collapsed_grid =
      wfc.generateCollapsedGrid(output_width - N + 1, output_height - N + 1);
  std::vector<uint8_t> output_pixels = overlapping_patterns.getIdsToPixels(
      collapsed_grid, output_width - N + 1, output_height - N + 1);

  // convert the output_pixels to an image and save it as a PNG file
  int channels = sprite.getChannels();
  stbi_write_png("../Results/output.png", output_width, output_height, channels,
                 output_pixels.data(), output_width * channels);

  return 0;
}
