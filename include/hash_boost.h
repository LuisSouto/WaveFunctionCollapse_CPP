#pragma once

#include <cstdint>

/**
 * A 64-bit hash combiner based on the Boost hash_combine structure,
 * updated with 64-bit Golden Ratio constants and SplitMix64 bit-shuffling.
 *
 * Sources:
 * - Boost Software License 1.0 (Structure)
 * - SplitMix64 by Steele et al. (Bit mixing)
 * - Golden Ratio fractional part (Entropy distribution)
 */
inline uint64_t hash_combine(uint64_t seed, uint32_t v) {
  const uint64_t k = 0x9e3779b97f4a7c15;
  uint64_t x = v;

  // SplitMix64 mixing
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
  x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
  x = x ^ (x >> 31);

  // Boost-style combination
  seed ^= x + k + (seed << 6) + (seed >> 2);

  return seed;
}