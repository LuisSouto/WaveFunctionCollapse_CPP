#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <unordered_map>
#include <vector>
#include <wfc_globals.h>
#include <wfc_typedefs.h>

class AdjacencyData {
private:
  std::vector<uint64_t> neighbour_ids;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<double> pattern_freq_times_log_freqs;
  std::vector<uint64_t> patterns_at_boundaries;
  std::vector<uint64_t> exclusively_boundary_patterns;
  std::vector<size_t> pattern_offsets;
  size_t num64_blocks;
  size_t num_blocks;
  size_t num_patterns;

  void findExclusivelyBoundaryPatterns();

public:
  AdjacencyData() = default;

  AdjacencyData(std::vector<std::unordered_map<pattern_id_t, uint64_t>>
                    &adjacent_patterns,
                std::vector<uint64_t> pattern_frequencies,
                std::vector<uint64_t> patterns_at_boundaries);

  size_t getNumPatterns() const { return num_patterns; }

  uint64_t getPatternFrequency(pattern_id_t pattern_id) {
    return pattern_frequencies[pattern_id];
  }

  double getPatternFreqTimesLogFreq(pattern_id_t pattern_id) {
    return pattern_freq_times_log_freqs[pattern_id];
  }

  size_t getNum64Blocks() const { return num64_blocks; }

  /* Constraints enforced by a given pattern in all directions */
  std::span<const uint64_t>
  getConstraintsFromPattern(pattern_id_t pattern_id) const {
    return {&neighbour_ids[pattern_offsets[pattern_id]], num_blocks};
  }

  /* Constraints enforced by a given pattern in a given direction */
  std::span<const uint64_t>
  getConstraintsFromPatternAtDirection(pattern_id_t pattern_id,
                                       uint8_t direction) const {

    size_t offset = (pattern_id * NUM_DIRECTIONS_2D + direction) * num64_blocks;
    return {&neighbour_ids[offset], num64_blocks};
  }

  /* Patterns present in the boundaries of the input image, indexed by
   * direction */
  std::span<const uint64_t> getPatternsAtBoundaries(size_t direction) const {
    return {&patterns_at_boundaries[direction * num64_blocks], num64_blocks};
  }

  std::span<const uint64_t> getExclusivelyBoundaryPatterns() const {
    return {exclusively_boundary_patterns.data(),
            exclusively_boundary_patterns.size()};
  }
};