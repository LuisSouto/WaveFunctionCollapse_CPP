#ifndef SPRITE_READER_H
#define SPRITE_READER_H

#include <sprite_holder.h>

class SpriteReader {
public:
  static SpriteHolder loadFromPng(const char *filename,
                                  const int &desired_channels);
};

#endif // SPRITE_READER_H