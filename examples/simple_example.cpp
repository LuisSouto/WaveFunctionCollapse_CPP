#include <chrono>
#include <overlapping_patterns.h>
#include <sprite_reader.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>
#include <wfc_core.h>

int main() {
  std::string filename = "../Sprites/Flowers.png";
  SpriteHolder sprite = SpriteReader::loadFromPng(filename.c_str(), STBI_rgb);
  size_t N = 2;
  OverlappingPatterns overlapping_patterns(sprite, N, BoundaryCondition::NONE);
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  // uint64_t seed = 5;
  WFCCore wfc(overlapping_patterns.getAdjacencyData(), seed);
  size_t output_width = 128;
  size_t output_height = 128;
  size_t adj_output_width = output_width - N + 1;
  size_t adj_output_height = output_height - N + 1;
  std::span<const pattern_id_t> collapsed_grid;
  // Random number generator for the starting index
  std::mt19937_64 rng(seed);
  std::uniform_int_distribution<size_t> dist(0, adj_output_width * adj_output_height - 1);

  // Generate images
  for (size_t i = 0; i < 100; i++) {
    collapsed_grid = wfc.solve(adj_output_width, adj_output_height, dist(rng), true,
                               CellSelectionStrategy::SCANLINE, {});
    std::vector<uint8_t> output_pixels = overlapping_patterns.convertIdsToPixels(
        collapsed_grid, adj_output_width, adj_output_height);

    // convert the output_pixels to an image and save it as a PNG file
    int channels = sprite.getChannels();
    std::string output_filename = "../Results/output" + std::to_string(i) + ".png";
    stbi_write_png(output_filename.c_str(), output_width, output_height, channels,
                   output_pixels.data(), output_width * channels);
  }

  // TODO: if a cell has tons of possible patterns, just assume it does not
  // restrict its neighbours
  // TODO: if neighbour_constraints is all 1's avoid intersection
  return 0;
}
