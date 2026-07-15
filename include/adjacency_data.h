#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include <wfc_globals.h>
#include <wfc_typedefs.h>

class AdjacencyData {
private:
  size_t num_patterns;
  size_t num_64_blocks;
  size_t num_blocks;
  std::vector<uint64_t> neighbour_ids;
  std::vector<uint64_t> pattern_frequencies;
  std::vector<double> pattern_freq_times_log_freqs;
  std::vector<uint64_t> patterns_at_boundaries;
  std::vector<size_t> pattern_offsets;

public:
  AdjacencyData() = default;
  AdjacencyData(std::vector<std::unordered_map<pattern_id_t, uint64_t>>
                    &adjacent_patterns,
                std::vector<uint64_t> pattern_frequencies,
                std::vector<uint64_t> patterns_at_boundaries);
  size_t getNumPatterns() const { return num_patterns; }
  size_t getNum64Blocks() const { return num_64_blocks; }
  uint64_t getPatternFrequency(pattern_id_t pattern_id) {
    return pattern_frequencies[pattern_id];
  }
  double getPatternFreqTimesLogFreq(pattern_id_t pattern_id) {
    return pattern_freq_times_log_freqs[pattern_id];
  }
  std::span<const uint64_t>
  getConstraintsForPatternAtDirection(pattern_id_t pattern_id,
                                      uint8_t direction) const {

    size_t offset =
        (pattern_id * NUM_DIRECTIONS_2D + direction) * num_64_blocks;
    return {&neighbour_ids[offset], num_64_blocks};
  }
  std::span<const uint64_t>
  getConstraintsForPattern(pattern_id_t pattern_id) const {
    return {&neighbour_ids[pattern_offsets[pattern_id]], num_blocks};
  }

  std::span<const uint64_t> getPatternsAtBoundaries(size_t direction) const {
    return {&patterns_at_boundaries[direction * num_64_blocks], num_64_blocks};
  }
};