#pragma once

#include <adjacency_data.h>
#include <sprite_holder.h>
#include <sys/types.h>
#include <wfc_settings.h>
#include <wfc_typedefs.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <unordered_map>
#include <vector>

/*Looks for patterns in a sample image using overlapping.*/
class OverlappingPatterns {
private:
	std::vector<uint64_t> pattern_frequencies;
	std::vector<uint64_t> patterns_at_boundaries;
	std::vector<pattern_hash_t> grid_pattern_hashes;
	std::vector<pattern_id_t> grid_pattern_ids;
	std::vector<uint8_t> ids_to_pixels;
	std::vector<SpriteHolder> transformed_sprites;
	std::unordered_map<pattern_hash_t, pattern_id_t> hashes_to_ids;
	size_t N;
	size_t channels;
	std::vector<size_t> widths;
	std::vector<size_t> heights;
	size_t grid_size;
	size_t total_transforms;
	void computeGridSize(const std::vector<SpriteHolder> &transformed_sprites,
			BoundaryCondition boundary_condition);
	void computePatternHashes(const std::vector<SpriteHolder> &transformed_sprites, size_t N,
			BoundaryCondition boundary_condition);
	void mapHashesToIds();
	void computeGridIds();
	void findBoundaryPatterns();
	void mapIdsToPixels(const std::vector<SpriteHolder> &transformed_sprites);
	void countPatterns();
	AdjacencyData generateAdjacentData() const;

public:
	OverlappingPatterns(const SpriteHolder &sprite, int N, BoundaryCondition boundary_condition,
			uint8_t transform_flags);

	AdjacencyData getAdjacencyData() const { return generateAdjacentData(); }

	size_t getNumPatterns() const { return hashes_to_ids.size(); }

	std::vector<uint8_t> convertIdsToPixels(
			std::span<const pattern_id_t> pattern_ids, size_t width, size_t height) const;

	std::vector<uint8_t> getInputPixelPatterns() const;
};
