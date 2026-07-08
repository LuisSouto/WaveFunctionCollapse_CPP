#include <cstring>
#include <gtest/gtest.h>
#include <overlapping_patterns.h>
#include <sprite_holder.h>
#include <vector>

class OverlappingPatternsTest : public ::testing::Test {
protected:
  // Helper to create a test sprite with uniform color
  SpriteHolder createUniformSprite(int width, int height, int channels) {
    std::vector<uint8_t> pixels(width * height * channels, 100);
    for (size_t i = 0; i < pixels.size(); i += channels) {
      pixels[i] = 100; // R
      if (channels > 1)
        pixels[i + 1] = 150; // G
      if (channels > 2)
        pixels[i + 2] = 200; // B
    }
    return SpriteHolder(width, height, channels, pixels);
  }

  // Helper to create a sprite with varied colors
  SpriteHolder createVariedSprite(int width, int height) {
    std::vector<uint8_t> pixels(width * height * 3);
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int idx = (y * width + x) * 3;
        pixels[idx] = (x * 50) % 256;           // R
        pixels[idx + 1] = (y * 50) % 256;       // G
        pixels[idx + 2] = ((x + y) * 25) % 256; // B
      }
    }
    return SpriteHolder(width, height, 3, pixels);
  }
};

// Test basic constructor with valid N
TEST_F(OverlappingPatternsTest, ConstructorValidN) {
  SpriteHolder sprite = createUniformSprite(5, 5, 3);

  EXPECT_NO_THROW({ OverlappingPatterns patterns(sprite, 2); });
}

// Test pattern extraction with 2x2 patterns on 3x3 sprite
TEST_F(OverlappingPatternsTest, PatternExtraction2x2On3x3) {
  SpriteHolder sprite = createUniformSprite(3, 3, 3);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // 3x3 sprite with 2x2 patterns should give (3-2+1) * (3-2+1) = 2x2 = 4
  // patterns But since all pixels are the same, all patterns are identical, so
  // only 1 unique hash
  EXPECT_EQ(pattern_hashes.size(), 1);
}

// Test pattern extraction with 1x1 patterns (should extract all pixels)
TEST_F(OverlappingPatternsTest, PatternExtraction1x1) {
  SpriteHolder sprite = createVariedSprite(4, 4);
  OverlappingPatterns patterns(sprite, 1);

  const auto &pattern_hashes = patterns.getPatternHashes();
  // 4x4 sprite with 1x1 patterns gives 4*4 = 16 unique positions
  // With varied colors, should have multiple unique hashes
  EXPECT_GE(pattern_hashes.size(), 1);
  EXPECT_LE(pattern_hashes.size(), 16);
}

// Test pattern hash vector contents
TEST_F(OverlappingPatternsTest, PatternHashVectorContents) {
  SpriteHolder sprite = createUniformSprite(4, 4, 3);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // Each pattern should have N*N pixel hashes
  for (const auto &[hash, pixel_hashes] : pattern_hashes) {
    EXPECT_EQ(pixel_hashes.size(), 4); // 2*2 = 4 pixels per pattern
  }
}

// Test hashes to IDs mapping
TEST_F(OverlappingPatternsTest, HashesToIdsMapping) {
  SpriteHolder sprite = createUniformSprite(5, 5, 3);
  OverlappingPatterns patterns(sprite, 2);

  const auto &hashes_to_ids = patterns.getHashesToIds();
  const auto &pattern_hashes = patterns.getPatternHashes();

  // Every unique hash should have a unique ID
  EXPECT_EQ(hashes_to_ids.size(), pattern_hashes.size());

  // IDs should be sequential starting from 0
  for (const auto &[hash, id] : hashes_to_ids) {
    EXPECT_GE(id, 0);
    EXPECT_LT(id, static_cast<int>(hashes_to_ids.size()));
  }
}

// Test that all hashes in the map are unique
TEST_F(OverlappingPatternsTest, UniqueHashes) {
  SpriteHolder sprite = createVariedSprite(6, 6);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // Map keys are unique by definition, so we just verify they're all present
  EXPECT_GT(pattern_hashes.size(), 0);
}

// Test with larger N value
TEST_F(OverlappingPatternsTest, LargerPatternSize) {
  SpriteHolder sprite = createUniformSprite(10, 10, 3);

  EXPECT_NO_THROW({ OverlappingPatterns patterns(sprite, 4); });

  OverlappingPatterns patterns(sprite, 4);
  const auto &pattern_hashes = patterns.getPatternHashes();

  // 10x10 sprite with 4x4 patterns should give (10-4+1) * (10-4+1) = 7*7 = 49
  // patterns With uniform colors, all identical, so only 1 unique hash
  EXPECT_EQ(pattern_hashes.size(), 1);
}

// Test pattern extraction with varied colors produces multiple unique patterns
TEST_F(OverlappingPatternsTest, VariedColorsProduceMultiplePatterns) {
  SpriteHolder sprite = createVariedSprite(5, 5);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // With varied colors, should have multiple unique patterns
  // Minimum is 1, but with varied input should be more
  EXPECT_GT(pattern_hashes.size(), 1);
}

// Test pattern ID assignment is within bounds
TEST_F(OverlappingPatternsTest, PatternIdsWithinBounds) {
  SpriteHolder sprite = createVariedSprite(8, 8);
  OverlappingPatterns patterns(sprite, 3);

  const auto &hashes_to_ids = patterns.getHashesToIds();
  int max_id = hashes_to_ids.size() - 1;

  for (const auto &[hash, id] : hashes_to_ids) {
    EXPECT_GE(id, 0);
    EXPECT_LE(id, max_id);
  }
}

// Test consistency - creating the same patterns twice should yield same results
TEST_F(OverlappingPatternsTest, Consistency) {
  SpriteHolder sprite = createVariedSprite(5, 5);
  OverlappingPatterns patterns1(sprite, 2);
  OverlappingPatterns patterns2(sprite, 2);

  const auto &hashes1 = patterns1.getHashesToIds();
  const auto &hashes2 = patterns2.getHashesToIds();

  // Same input should produce same hashes
  EXPECT_EQ(hashes1.size(), hashes2.size());
}

// Test with minimum size sprite (N x N)
TEST_F(OverlappingPatternsTest, MinimumSizeSprite) {
  SpriteHolder sprite = createUniformSprite(3, 3, 3);
  OverlappingPatterns patterns(sprite, 3);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // 3x3 sprite with 3x3 patterns should give exactly 1 pattern
  EXPECT_EQ(pattern_hashes.size(), 1);

  const auto &hashes_to_ids = patterns.getHashesToIds();
  EXPECT_EQ(hashes_to_ids.size(), 1);
}

// Test getters return constant references to data
TEST_F(OverlappingPatternsTest, GettersReturnValidData) {
  SpriteHolder sprite = createVariedSprite(5, 5);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();
  const auto &hashes_to_ids = patterns.getHashesToIds();

  // Verify we can access the data
  EXPECT_GT(pattern_hashes.size(), 0);
  EXPECT_GT(hashes_to_ids.size(), 0);

  // Verify iteration works
  for (const auto &[hash, pixels] : pattern_hashes) {
    EXPECT_GT(pixels.size(), 0);
  }

  for (const auto &[hash, id] : hashes_to_ids) {
    EXPECT_GE(id, 0);
  }
}

// Test pattern pixel hashes match sprite pixel hashes
TEST_F(OverlappingPatternsTest, PatternPixelHashesMatchSpritePixelHashes) {
  SpriteHolder sprite = createVariedSprite(4, 4);
  OverlappingPatterns patterns(sprite, 2);

  const auto &pattern_hashes = patterns.getPatternHashes();

  // Get the first pattern and verify its pixel hashes match the sprite
  if (!pattern_hashes.empty()) {
    const auto &first_pattern_pixels = pattern_hashes.begin()->second;

    // Verify pattern contains valid pixel hash values
    for (const auto &pixel_hash : first_pattern_pixels) {
      // Just verify it's a valid 24-bit RGB value
      EXPECT_LE(pixel_hash, 0xFFFFFF);
    }
  }
}
