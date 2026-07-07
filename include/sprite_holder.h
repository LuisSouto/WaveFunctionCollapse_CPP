class SpriteHolder {
private:
  int width;
  int height;
  int channels;
  int *image_pixels;

public:
  SpriteHolder(int width, int height, int channels, int *image_pixels)
      : width(width), height(height), channels(channels),
        image_pixels(image_pixels) {};
};