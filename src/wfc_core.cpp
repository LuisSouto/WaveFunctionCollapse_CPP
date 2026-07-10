#include "wfc_typedefs.h"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>
#include <wfc_core.h>

void WFC::run(size_t output_width, size_t output_height) {
  initializeGrid(output_width, output_height);
  size_t current_cell_index = 0;
  while (true) {
    current_cell_index = findCellToCollapse(current_cell_index);
    collapsePatternForCell(current_cell_index);
    propagateConstraints();
  }
}

void WFC::initializeGrid(size_t output_width, size_t output_height) {
  this->output_width = output_width;
  this->output_height = output_height;

  // First use 1's to mark every pattern as possible
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  grid.assign(output_width * output_height * num_64_blocks, ~0ULL);

  // Clear the bits that are beyond the number of patterns
  size_t bits_to_remove = adjacent_data.getNumPatterns() % 64;
  if (bits_to_remove > 0) {
    size_t last_block_offset = num_64_blocks - 1;
    uint64_t tail_mask = (1ULL << (64 - bits_to_remove)) - 1;

    for (size_t i = 0; i < output_width * output_height; i++) {
      size_t last_block_index = i * num_64_blocks + last_block_offset;
      grid[last_block_index] = tail_mask;
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
  pattern_frequencies.reserve(possible_pattern_ids.size());
  for (pattern_id_t id : possible_pattern_ids) {
    pattern_frequencies.push_back(adjacent_data.getPatternFrequency(id));
  }

  std::discrete_distribution<size_t> dist(pattern_frequencies.begin(),
                                          pattern_frequencies.end());

  // Sample a possible pattern and collapse the cell
  pattern_id_t selected_pattern_id = possible_pattern_ids[dist(rng)];
  for (size_t i = 0; i < num_64_blocks; i++) {
    grid[start_index + i] = 0;
  }

  size_t target_block = selected_pattern_id / 64;
  int target_bit = selected_pattern_id % 64;
  grid[start_index + target_block] = (1ULL << target_bit);
}

void WFC::findPossiblePatternsAtCell(size_t cell_index) {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;

  // Preallocate memory to avoid reallocations
  size_t total_bits = 0;
  for (size_t i = 0; i < num_64_blocks; i++) {
    total_bits += std::popcount(grid[start_index + i]);
  }

  if (total_bits == 0) {
    throw std::runtime_error(
        "Contradiction detected: Cell has no possible patterns.");
  }

  possible_pattern_ids.clear();
  possible_pattern_ids.reserve(total_bits);

  // Find all the significant bits in the 64-bit blocks
  for (size_t i = 0; i < num_64_blocks; i++) {
    uint64_t block = grid[start_index + i];
    pattern_id_t block_base_id = i * 64; // Accumulated id for each block
    while (block != 0) {
      pattern_id_t zeros_until_id = std::countr_zero(block);
      possible_pattern_ids.push_back(block_base_id + zeros_until_id);
      block &= (block - 1); // Clear the least significant bit set
    }
  }
}