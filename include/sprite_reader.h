#include <sprite_holder.h>

class SpriteReader {
public:
  static SpriteHolder loadFromPng(const char *filename,
                                  const int &desired_channels);
};