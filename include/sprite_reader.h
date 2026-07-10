#pragma once

#include <sprite_holder.h>

class SpriteReader {
public:
  static SpriteHolder loadFromPng(const char *filename, int desired_channels);
};