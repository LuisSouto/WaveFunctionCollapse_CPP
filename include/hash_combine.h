#pragma once

#include "wfc_typedefs.h"
#include <cstdint>

/* Combine hashes into a single hash. The mixer algorithm is authored by Jon Maiga
 * (https://github.com/jonmaiga/mx3/tree/master) while the idea of using it to combine hashes was
 * taken from the Boost library
 * (https://www.boost.org/doc/libs/latest/libs/container_hash/doc/html/hash.html#notes_hash_combine)
 */
inline uint64_t hash_combine(uint64_t seed, pixel_hash_t v) {
  const uint64_t k = 0x9e3779b97f4a7c15;
  uint64_t x = seed + k + v;

  x ^= x >> 32;
  x *= 0xbea225f9eb34556d;
  x ^= x >> 29;
  x *= 0xbea225f9eb34556d;
  x ^= x >> 32;
  x *= 0xbea225f9eb34556d;
  x ^= x >> 29;

  return x;
}