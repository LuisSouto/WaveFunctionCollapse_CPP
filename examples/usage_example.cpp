#include "overlapping_patterns.h"
#include "sprite_reader.h"
#include "sprite_transforms.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "wfc_core.h"
#include "wfc_typedefs.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

int main() {
  // First we load the sprite as pixel data
  std::string filename = "./assets/twisted_paths.png";
  SpriteHolder sprite = SpriteReader::loadFromPng(filename.c_str(), STBI_rgb_alpha);

  // Extract the patterns present in the image
  size_t N = 3; // Pattern length (typically 2 or 3 works best)
  OverlappingPatterns overlapping_patterns(sprite, N, BoundaryCondition::NONE,
                                           SpriteTransforms::ALL_TRANSFORMS);

  // Initialize WFC, you can use a fixed seed for reproducible results
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  WFCCore wfc(overlapping_patterns.generateAdjacencyData(), seed);

  // Output image and WFC grid dimensions
  size_t output_width = 32;
  size_t output_height = 32;
  size_t grid_width = output_width - N + 1;
  size_t grid_height = output_height - N + 1;
  std::span<const pattern_id_t> collapsed_grid;

  // Generate images
  size_t num_images = 1;
  int start_index = -1; // negative index means the algorithm will randomly choose a start point
  bool force_boundary_patterns = false; // if true, enforce boundaries (e.g. walls) around the image
  std::unordered_map<size_t, pattern_id_t> fixed_cells = {};
  for (size_t i = 0; i < num_images; ++i) {
    // Call WFC to generate the output image. The algorithm returns a grid where each element
    // corresponds to a pattern identifier, so not yet in pixel form.
    collapsed_grid = wfc.solve(grid_width, grid_height, start_index, force_boundary_patterns,
                               CellSelectionStrategy::SCANLINE, fixed_cells);

    // Make sure to check the algorithm finished successfully
    if (collapsed_grid.empty()) {
      std::cout << "Output image could not be generated: failed too many times." << std::endl;
      continue;
    }

    // Convert pattern identifiers back to pixels
    std::vector<uint8_t> output_pixels =
        overlapping_patterns.convertIdsToPixels(collapsed_grid, grid_width, grid_height);

    // Save image, stbi supports other formats appart from .png so adjust to your use case
    int channels = sprite.getChannels();
    std::string output_filename = "../Results/output" + std::to_string(i) + ".png";
    stbi_write_png(output_filename.c_str(), output_width, output_height, channels,
                   output_pixels.data(), output_width * channels);
  }

  return 0;
}
