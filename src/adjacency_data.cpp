#include <adjacency_data.h>
#include <utility>
#include <wfc_globals.h>

AdjacencyData::AdjacencyData(
    std::vector<std::unordered_map<pattern_id_t, uint64_t>> &adjacent_patterns,
    std::vector<uint64_t> pattern_frequencies,
    std::vector<uint64_t> patterns_at_boundaries) {

  num_patterns = adjacent_patterns.size() / NUM_DIRECTIONS_2D;
  num_64_blocks = (num_patterns + 63) / 64; // Round up to nearest mult of 64
  num_blocks = num_64_blocks * NUM_DIRECTIONS_2D;

  neighbour_ids.clear();
  neighbour_ids.resize(adjacent_patterns.size() * num_64_blocks, 0);
  this->pattern_frequencies = std::move(pattern_frequencies);
  this->patterns_at_boundaries = std::move(patterns_at_boundaries);
  pattern_offsets.reserve(num_patterns);

  for (size_t pattern_id = 0; pattern_id < num_patterns; pattern_id++) {
    pattern_offsets.push_back(pattern_id * num_blocks);
    for (size_t direction = 0; direction < NUM_DIRECTIONS_2D; direction++) {
      size_t map_index = pattern_id * NUM_DIRECTIONS_2D + direction;
      for (const auto &[neighbour_pattern_id, frequency] :
           adjacent_patterns[map_index]) {
        size_t block_index = neighbour_pattern_id / 64;
        size_t bit_index = neighbour_pattern_id % 64;
        neighbour_ids[map_index * num_64_blocks + block_index] |=
            (1ULL << bit_index);
      }
    }
  }
}