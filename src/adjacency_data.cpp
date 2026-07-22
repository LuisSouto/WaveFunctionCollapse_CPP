#include <adjacency_data.h>
#include <math.h>
#include <wfc_globals.h>

AdjacencyData::AdjacencyData(
    std::vector<std::unordered_map<pattern_id_t, uint64_t>> &adjacent_patterns,
    std::vector<uint64_t> pattern_frequencies,
    std::vector<uint64_t> patterns_at_boundaries) {

  num_patterns = adjacent_patterns.size() / NUM_DIRECTIONS_2D;
  num64_blocks = (num_patterns + 63) / 64; // Round up to nearest mult of 64
  num_blocks = num64_blocks * NUM_DIRECTIONS_2D;

  neighbour_ids.clear();
  neighbour_ids.resize(adjacent_patterns.size() * num64_blocks, 0);
  this->pattern_frequencies = std::vector<uint64_t>(pattern_frequencies.begin(),
                                                    pattern_frequencies.end());
  pattern_freq_times_log_freqs.resize(this->pattern_frequencies.size(), 0);
  for (size_t i = 0; i < this->pattern_frequencies.size(); i++) {
    uint64_t freq = this->pattern_frequencies[i];
    if (freq > 0) {
      double log_freq = std::log(static_cast<double>(freq));
      pattern_freq_times_log_freqs[i] = freq * log_freq;
    }
  }

  this->patterns_at_boundaries = std::vector<uint64_t>(
      patterns_at_boundaries.begin(), patterns_at_boundaries.end());

  pattern_offsets.reserve(num_patterns);
  for (size_t pattern_id = 0; pattern_id < num_patterns; pattern_id++) {
    pattern_offsets.push_back(pattern_id * num_blocks);
    for (size_t direction = 0; direction < NUM_DIRECTIONS_2D; direction++) {
      size_t map_index = pattern_id * NUM_DIRECTIONS_2D + direction;
      for (const auto &[neighbour_pattern_id, frequency] :
           adjacent_patterns[map_index]) {
        size_t block_index = neighbour_pattern_id / 64;
        size_t bit_index = neighbour_pattern_id % 64;
        neighbour_ids[map_index * num64_blocks + block_index] |=
            (1ULL << bit_index);
      }
    }
  }

  findExclusivelyBoundaryPatterns();
}

void AdjacencyData::findExclusivelyBoundaryPatterns() {
  exclusively_boundary_patterns.clear();
  exclusively_boundary_patterns.resize(num64_blocks, 0);

  for (size_t direction = 0; direction < NUM_DIRECTIONS_2D; ++direction) {
    for (size_t block_index = 0; block_index < num64_blocks; ++block_index) {
      size_t index = direction * num64_blocks + block_index;
      uint64_t boundary_block = patterns_at_boundaries[index];
      uint64_t exclude_pattern_block = 0;
      while (boundary_block != 0) {
        size_t pattern_id = std::countr_zero(boundary_block);
        bool exclude_pattern = true;
        for (size_t i = 0; i < num64_blocks; ++i) {
          if (neighbour_ids[(pattern_id * NUM_DIRECTIONS_2D + direction) *
                                num64_blocks +
                            i] != 0) {
            exclude_pattern = false;
            break;
          }
        }
        if (exclude_pattern) {
          exclude_pattern_block |= (1ULL << pattern_id);
        }
        boundary_block &=
            (boundary_block - 1); // Clear the least significant bit set
      }
      exclusively_boundary_patterns[block_index] |= exclude_pattern_block;
    }
  }
}