#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <wfc_typedefs.h>

class AdjacencyData {
private:
  std::vector<pattern_id_t> neighbour_ids;
  std::vector<uint64_t> neighbour_frequencies;
  // For each pattern and direction, store the offsets of its adjacent patterns
  // in the grid
  std::vector<size_t> neighbour_offsets;
  // For each pattern and direction, store the count of its adjacent patterns
  std::vector<size_t> neighbour_counts;

public:
  AdjacencyData() = default;
  AdjacencyData(
      std::vector<std::unordered_map<pattern_id_t, uint64_t>> &discovered_maps,
      size_t width, size_t height);
};