#include <iostream>
#include <sprite_reader.h>
#include <stb_image.h>
#include <vector>

SpriteHolder SpriteReader::loadFromPng(const char *filename,
                                       const int &desired_channels) {
  int width;
  int height;
  int n_channels;
  unsigned char *image_data =
      stbi_load(filename, &width, &height, &n_channels, desired_channels);

  for (int i = 0; i < width * height; i++)
    std::cout << (int)image_data[i] << std::endl;

  // Make into flat array
  size_t image_size = width * height * desired_channels;
  std::vector<int> image_pixels_rgb(image_size);

  for (int i = 0; i < image_size; i++) {
    image_pixels_rgb[i] = (int)image_data[i];
  }
  stbi_image_free(image_data);

  return SpriteHolder(width, height, desired_channels, image_pixels_rgb.data());
}