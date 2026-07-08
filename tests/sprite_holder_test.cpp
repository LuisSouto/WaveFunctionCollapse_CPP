#include <cstring>
#include <gtest/gtest.h>
#include <sprite_holder.h>
#include <vector>

class SpriteHolderTest : public ::testing::Test {
protected:
  // Helper to create a simple test sprite with known pixel values
  SpriteHolder createTestSprite(int width, int height, int channels) {
    std::vector<uint8_t> pixels(width * height * channels);
    // Fill with a simple pattern: R=100, G=150, B=200
    for (size_t i = 0; i < pixels.size(); i += channels) {
      pixels[i] = 100; // R
      if (channels > 1)
        pixels[i + 1] = 150; // G
      if (channels > 2)
        pixels[i + 2] = 200; // B
      if (channels > 3)
        pixels[i + 3] = 255; // A
    }
    return SpriteHolder(width, height, channels, pixels);
  }
};

// Test basic constructor and getters
TEST_F(SpriteHolderTest, ConstructorAndGetters) {
  SpriteHolder sprite = createTestSprite(10, 10, 3);

  EXPECT_EQ(sprite.getWidth(), 10);
  EXPECT_EQ(sprite.getHeight(), 10);
  EXPECT_EQ(sprite.getChannels(), 3);
}

// Test pixel data is correctly stored
TEST_F(SpriteHolderTest, PixelDataStorage) {
  std::vector<uint8_t> pixels = {100, 150, 200, 50, 75, 125};
  SpriteHolder sprite(2, 1, 3, pixels);

  const auto &stored_pixels = sprite.getImagePixels();
  EXPECT_EQ(stored_pixels.size(), pixels.size());
  for (size_t i = 0; i < pixels.size(); ++i) {
    EXPECT_EQ(stored_pixels[i], pixels[i]);
  }
}

// Test pixel hash computation for RGB
TEST_F(SpriteHolderTest, PixelHashComputationRGB) {
  std::vector<uint8_t> pixels = {100, 150, 200}; // R=100, G=150, B=200
  SpriteHolder sprite(1, 1, 3, pixels);

  uint32_t expected_hash = 100 | (150 << 8) | (200 << 16);
  EXPECT_EQ(sprite.getPixelHash(0, 0), expected_hash);
}

// Test pixel hash for multi-pixel image
TEST_F(SpriteHolderTest, PixelHashMultiPixel) {
  std::vector<uint8_t> pixels = {
      100, 150, 200, // Pixel (0,0): R=100, G=150, B=200
      50,  75,  125  // Pixel (1,0): R=50, G=75, B=125
  };
  SpriteHolder sprite(2, 1, 3, pixels);

  uint32_t hash1 = sprite.getPixelHash(0, 0);
  uint32_t hash2 = sprite.getPixelHash(1, 0);

  uint32_t expected_hash1 = 100 | (150 << 8) | (200 << 16);
  uint32_t expected_hash2 = 50 | (75 << 8) | (125 << 16);

  EXPECT_EQ(hash1, expected_hash1);
  EXPECT_EQ(hash2, expected_hash2);
  EXPECT_NE(hash1, hash2);
}

// Test pixel hash with RGBA
TEST_F(SpriteHolderTest, PixelHashRGBA) {
  std::vector<uint8_t> pixels = {100, 150, 200, 255}; // RGBA
  SpriteHolder sprite(1, 1, 4, pixels);

  // Alpha channel is ignored in hash computation (only RGB)
  uint32_t expected_hash = 100 | (150 << 8) | (200 << 16);
  EXPECT_EQ(sprite.getPixelHash(0, 0), expected_hash);
}

// Test boundary conditions - valid coordinates at edges
TEST_F(SpriteHolderTest, BoundaryCoordinatesValid) {
  SpriteHolder sprite = createTestSprite(5, 5, 3);

  // Should not throw for valid edge coordinates
  EXPECT_NO_THROW(sprite.getPixelHash(0, 0)); // Top-left
  EXPECT_NO_THROW(sprite.getPixelHash(4, 4)); // Bottom-right
  EXPECT_NO_THROW(sprite.getPixelHash(0, 4)); // Top-right
  EXPECT_NO_THROW(sprite.getPixelHash(4, 0)); // Bottom-left
}

// Test out of bounds - x coordinate
TEST_F(SpriteHolderTest, OutOfBoundsX) {
  SpriteHolder sprite = createTestSprite(5, 5, 3);

  EXPECT_THROW(sprite.getPixelHash(-1, 0), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(5, 0), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(10, 0), std::out_of_range);
}

// Test out of bounds - y coordinate
TEST_F(SpriteHolderTest, OutOfBoundsY) {
  SpriteHolder sprite = createTestSprite(5, 5, 3);

  EXPECT_THROW(sprite.getPixelHash(0, -1), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(0, 5), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(0, 10), std::out_of_range);
}

// Test out of bounds - both coordinates
TEST_F(SpriteHolderTest, OutOfBoundsBoth) {
  SpriteHolder sprite = createTestSprite(5, 5, 3);

  EXPECT_THROW(sprite.getPixelHash(-1, -1), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(5, 5), std::out_of_range);
  EXPECT_THROW(sprite.getPixelHash(-5, -5), std::out_of_range);
}

// Test pixel hashes vector
TEST_F(SpriteHolderTest, PixelHashesVector) {
  SpriteHolder sprite = createTestSprite(3, 2, 3);

  const auto &hashes = sprite.getPixelHashes();
  EXPECT_EQ(hashes.size(), 6); // 3 * 2 pixels

  // All hashes should be the same since all pixels have the same color
  for (size_t i = 1; i < hashes.size(); ++i) {
    EXPECT_EQ(hashes[i], hashes[0]);
  }
}

// Test different pixel colors produce different hashes
TEST_F(SpriteHolderTest, DifferentColorsProduceDifferentHashes) {
  std::vector<uint8_t> pixels = {
      100, 150, 200, // Pixel 1
      101, 150, 200  // Pixel 2 - R differs by 1
  };
  SpriteHolder sprite(2, 1, 3, pixels);

  EXPECT_NE(sprite.getPixelHash(0, 0), sprite.getPixelHash(1, 0));
}

// Test 1x1 sprite
TEST_F(SpriteHolderTest, SinglePixelSprite) {
  std::vector<uint8_t> pixels = {255, 128, 64};
  SpriteHolder sprite(1, 1, 3, pixels);

  EXPECT_EQ(sprite.getWidth(), 1);
  EXPECT_EQ(sprite.getHeight(), 1);
  uint32_t expected = 255 | (128 << 8) | (64 << 16);
  EXPECT_EQ(sprite.getPixelHash(0, 0), expected);
}
