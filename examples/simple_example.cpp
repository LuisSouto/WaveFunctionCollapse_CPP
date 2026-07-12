#include <iostream>
#include <overlapping_patterns.h>
#include <sprite_reader.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>
#include <wfc_core.h>

int main() {
  std::string filename = "../Sprites/flat_blue.png";
  SpriteHolder sprite = SpriteReader::loadFromPng(filename.c_str(), STBI_rgb);
  size_t N = 3;
  OverlappingPatterns overlapping_patterns(sprite, N);
  uint64_t seed = 123456789;
  WFC wfc(overlapping_patterns.getAdjacencyData(), seed);
  size_t output_width = 128;
  size_t output_height = 128;
  std::span<const pattern_id_t> collapsed_grid =
      wfc.generateCollapsedGrid(output_width - N + 1, output_height - N + 1);
  std::vector<uint8_t> output_pixels = overlapping_patterns.getIdsToPixels(
      collapsed_grid, output_width - N + 1, output_height - N + 1);
  for (size_t i = 0; i < output_pixels.size(); i++) {
    std::cout << static_cast<int>(output_pixels[i]) << " ";
  }
  std::cout << std::endl;
  std::cout << output_pixels.size() << std::endl;

  // convert the output_pixels to an image and save it as a PNG file
  int channels = sprite.getChannels();
  stbi_write_png("../Results/output.png", output_width, output_height, channels,
                 output_pixels.data(), output_width * channels);

  return 0;
}
