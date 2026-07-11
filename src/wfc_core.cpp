#include "wfc_typedefs.h"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <directions.h>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>
#include <wfc_core.h>
#include <wfc_globals.h>

void WFC::run(size_t output_width, size_t output_height) {
  total_cells = output_height * output_width;
  size_t num64_blocks = adjacent_data.getNum64Blocks();

  // Initialize grid and other variables
  initializeGrid(output_width, output_height);
  cell_update_versions.resize(total_cells, 0);
  current_cell_wave.resize(total_cells);
  combined_neighbour_patterns.resize(adjacent_data.getNum64Blocks());
  cell_possible_pattern_ids.reserve(num64_blocks * 64);
  pattern_frequencies.reserve(num64_blocks * 64);
  current_version = 0;
  direction_offsets = {-1, 1, -static_cast<int>(output_width),
                       static_cast<int>(output_width)};

  // Collapse cells until the whole grid is collapsed
  size_t current_cell_index = 0;
  collapsed_cells = 0;
  while (collapsed_cells < total_cells) {
    collapsePatternForCell(current_cell_index);
    propagateConstraints(current_cell_index);
    current_cell_index = findCellToCollapse(current_cell_index);
  }
}

void WFC::initializeGrid(size_t output_width, size_t output_height) {
  this->output_width = output_width;
  this->output_height = output_height;

  // First use 1's to mark every pattern as possible
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  grid.assign(total_cells * num_64_blocks, ~0ULL);

  // Clear the bits that are beyond the number of patterns
  size_t bits_to_remove = adjacent_data.getNumPatterns() % 64;
  if (bits_to_remove > 0) {
    size_t last_block_offset = num_64_blocks - 1;
    uint64_t tail_mask = (1ULL << (64 - bits_to_remove)) - 1;

    size_t last_block_index = last_block_offset;
    for (size_t i = 0; i < total_cells; i++) {
      grid[last_block_index] = tail_mask;
      last_block_index += num_64_blocks;
    }
  }
}

size_t WFC::findCellToCollapse(size_t previous_cell_index) {
  size_t previous_x = previous_cell_index % output_width;

  if (previous_x + 1 < output_width) {
    return previous_cell_index + 1;
  } else {
    size_t next_y = (previous_cell_index / output_width) + 1;
    if (next_y < output_height) {
      return next_y * output_width;
    } else {
      // Reached the end of the grid
      return SIZE_MAX;
    }
  }
}

void WFC::collapsePatternForCell(size_t cell_index) {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;

  findPossiblePatternsAtCell(cell_index);

  // Find the frequency count of each pattern to create a discrete distribution
  pattern_frequencies.clear();
  for (pattern_id_t id : cell_possible_pattern_ids) {
    pattern_frequencies.push_back(adjacent_data.getPatternFrequency(id));
  }

  std::discrete_distribution<size_t> dist(pattern_frequencies.begin(),
                                          pattern_frequencies.end());

  // Sample a possible pattern and collapse the cell
  pattern_id_t selected_pattern_id = cell_possible_pattern_ids[dist(rng)];
  for (size_t i = 0; i < num_64_blocks; i++) {
    grid[start_index + i] = 0;
  }

  size_t target_block = selected_pattern_id / 64;
  int target_bit = selected_pattern_id % 64;
  grid[start_index + target_block] = (1ULL << target_bit);
  collapsed_cells++;
}

void WFC::findPossiblePatternsAtCell(size_t cell_index) {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;

  // Find all the significant bits in the 64-bit blocks
  cell_possible_pattern_ids.clear();
  for (size_t i = 0; i < num_64_blocks; i++) {
    uint64_t block = grid[start_index + i];
    if (block == 0)
      continue;

    pattern_id_t block_base_id = i * 64; // Accumulated id for each block
    while (block != 0) {
      pattern_id_t zeros_until_id = std::countr_zero(block);
      cell_possible_pattern_ids.push_back(block_base_id + zeros_until_id);
      block &= (block - 1); // Clear the least significant bit set
    }
  }

  if (cell_possible_pattern_ids.size() == 0) {
    throw std::runtime_error("Contradiction: no possible patterns found!");
  }
}

void WFC::propagateConstraints(size_t cell_index) {
  current_version++;
  current_cell_wave.clear();
  current_cell_wave.push_back(cell_index);
  cell_update_versions[cell_index] = current_version;
  while (current_cell_wave.size() > 0) {
    extendPropagationRange();
  }
}

void WFC::findPossibleNeighbourPatterns(size_t cell_index, uint8_t direction) {
  if (direction >= NUM_DIRECTIONS_2D)
    throw std::runtime_error("Unknown direction.");

  findPossiblePatternsAtCell(cell_index);
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  std::fill(combined_neighbour_patterns.begin(),
            combined_neighbour_patterns.end(), 0U);

  // Support of the union of the possible neighbours of every possible pattern
  // in the current cell
  for (pattern_id_t pattern_id : cell_possible_pattern_ids) {
    std::span<const uint64_t> neighbour_ids =
        adjacent_data.getNeighbourIds(pattern_id, direction);

    for (size_t i = 0; i < num_64_blocks; i++) {
      combined_neighbour_patterns[i] |= neighbour_ids[i];
    }
  }
}

void WFC::extendPropagationRange() {
  size_t wave_size = 0ULL;
  for (size_t cell_index : current_cell_wave) {
    // Check every direction
    for (int direction : direction_offsets) {
      size_t neighbour_index = cell_index + direction;
      // Out of bounds or already updated in this iteration
      if (!isCellWithinBoundaries(neighbour_index) ||
          cell_update_versions[neighbour_index] == current_version) {
        continue;
      }
    }
    // First check if within boundaries
    // Then check if it's using latests version. Ignore in that case
    // Otherwise compare it's possible patterns with the new list
    // If they match update it's version but do not add to wave
    // If they don't match we add it to the wave and increase counter
  }
  // It needs the visited cells so far (at first just the collapsed cell)
  // It should expand visited cells to the next level by looking in all
  // direction It should return a new list of cells to explore in the next
  // iteration Should check boundaries Do not add cells whose list of patterns
  // did not change
}

bool WFC::isCellWithinBoundaries(size_t cell_index) {
  size_t y = cell_index / output_width;
  size_t x = cell_index - (y * output_width);
  return (x < output_width && y < output_height);
}