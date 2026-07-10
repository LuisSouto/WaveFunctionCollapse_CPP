#include <adjacency_data.h>
#include <wfc_globals.h>

AdjacencyData::AdjacencyData(
    std::vector<std::unordered_map<pattern_id_t, uint64_t>> &discovered_maps,
    size_t width, size_t height) {

  num_patterns = discovered_maps.size() / NUM_DIRECTIONS_2D;
  num_64_blocks = (num_patterns + 63) / 64; // Round up to nearest mult of 64
  size_t total_entries = 0;
  for (const auto &map : discovered_maps) {
    total_entries += map.size();
  }
  neighbour_ids.clear();
  neighbour_ids.reserve(discovered_maps.size() * num_64_blocks);
  pattern_frequencies.clear();
  pattern_frequencies.reserve(total_entries);
  neighbour_offsets.clear();
  neighbour_offsets.reserve(discovered_maps.size());
}