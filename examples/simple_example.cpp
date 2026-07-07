#include <iostream>
#include <sprite_reader.h>
#include <stb_image.h>

int main() {
  std::string filename = "../Sprites/simple_road.png";
  SpriteHolder sprite = SpriteReader::loadFromPng(filename.c_str(), STBI_rgb);

  return 0;
}
