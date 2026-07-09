#include <adjacency_data.h>

AdjacencyData::AdjacencyData(
    std::vector<std::unordered_map<pattern_id_t, uint64_t>> &discovered_maps,
    size_t width, size_t height) {

  size_t total_entries = 0;
  for (const auto &map : discovered_maps) {
    total_entries += map.size();
  }
  neighbour_ids.reserve(total_entries);
  neighbour_frequencies.reserve(total_entries);
  neighbour_offsets.reserve(discovered_maps.size());
}