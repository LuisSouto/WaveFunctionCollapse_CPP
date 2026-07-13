#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <directions.h>
#include <iostream>
#include <random>
#include <span>
#include <vector>
#include <wfc_core.h>
#include <wfc_globals.h>
#include <wfc_typedefs.h>

std::span<const pattern_id_t> WFC::generateCollapsedGrid(size_t output_width,
                                                         size_t output_height) {
  total_cells = output_height * output_width;
  size_t num64_blocks = adjacent_data.getNum64Blocks();

  // Initialize grid and other variables
  initializeGrid(output_width, output_height);
  bakeNeighbourIndexes();
  temp_buffers.cell_update_versions.resize(total_cells, 0);
  temp_buffers.current_cell_wave.resize(total_cells);
  temp_buffers.next_cell_wave.resize(total_cells);
  temp_buffers.constraints_for_neighbour.resize(num64_blocks, 0);
  temp_buffers.constraints_from_cell.resize(num64_blocks * NUM_DIRECTIONS_2D,
                                            0);
  temp_buffers.current_version = 0;
  cell_pattern_ids.reserve(num64_blocks * 64);
  pattern_frequencies.reserve(num64_blocks * 64);

  if (!settings.enable_backtracking) {
    settings.block_size_x = output_width;
    settings.block_size_y = output_height;
  }

  current_block_x = 0;
  current_block_y = 0;
  backtracking_block.resize(
      settings.block_size_x * settings.block_size_y * num64_blocks, 0);
  is_outside_block.resize(total_cells, 1);
  num_collapsed_cells = 0;
  updateBackTrackingBlock();

  // Collapse cells until the whole grid is collapsed
  size_t current_cell_index = 0;
  size_t block_index_counter = 0;
  uint8_t no_contradictions;
  while (num_collapsed_cells < total_cells) {
    collapsePatternAtCell(current_cell_index);
    std::cout << "Collapsed " << num_collapsed_cells << " out of "
              << total_cells << " cells." << std::endl;
    no_contradictions = propagateConstraints(current_cell_index, false);
    if (!no_contradictions) {
      current_cell_index = restartBlock();
      continue;
    }
    block_index_counter++;
    if (block_index_counter >= settings.block_size_x * settings.block_size_y) {
      block_index_counter = 0;
      if (num_collapsed_cells >= total_cells) {
        break; // All cells are collapsed
      }
      no_contradictions = propagateConstraints(current_cell_index, true);
      if (no_contradictions) {
        current_cell_index = moveToNextBlock();
        propagateConstraints(current_cell_index - 1, false);
      } else {
        current_cell_index = restartBlock();
      }
      continue;
    }
    current_cell_index = findCellToCollapse(current_cell_index);
  }

  return {collapsed_patterns.data(), collapsed_patterns.size()};
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
    uint64_t tail_mask = (1ULL << bits_to_remove) - 1;

    size_t last_block_index = last_block_offset;
    for (size_t i = 0; i < total_cells; i++) {
      grid[last_block_index] = tail_mask;
      last_block_index += num_64_blocks;
    }
  }

  is_cell_collapsed.assign(total_cells, 0);
  collapsed_patterns.assign(total_cells, 0);
}

void WFC::bakeNeighbourIndexes() {
  // Bake the neighbour indexes for each cell to avoid recalculating them during
  // propagation
  neighbour_indexes.resize(total_cells * NUM_DIRECTIONS_2D);
  for (size_t cell_index = 0; cell_index < total_cells; cell_index++) {
    size_t x = cell_index % output_width;
    size_t y = cell_index / output_width;
    for (size_t i = 0; i < NUM_DIRECTIONS_2D; i++) {
      int dx = DX[i];
      int dy = DY[i];
      if (y + dy < 0 || y + dy >= output_height) {
        neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = SIZE_MAX;
        continue; // Out of bounds
      }
      if (x + dx < 0 || x + dx >= output_width) {
        neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = SIZE_MAX;
        continue; // Out of bounds
      }
      size_t neighbour_index = cell_index + dy * output_width + dx;
      neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = neighbour_index;
    }
  }
}

void WFC::updateBackTrackingBlock() {
  uint32_t block_size_x = settings.block_size_x;
  uint32_t block_size_y = settings.block_size_y;
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t current_block_index = current_block_y * output_width + current_block_x;
  for (size_t y = 0; y < block_size_y; y++) {
    for (size_t x = 0; x < block_size_x; x++) {
      size_t start_index_grid =
          (current_block_index + y * output_width + x) * num_64_blocks;
      size_t start_index_block = (y * block_size_x + x) * num_64_blocks;
      for (size_t k = 0; k < num_64_blocks; k++) {
        backtracking_block[start_index_block + k] = grid[start_index_grid + k];
      }
      is_outside_block[current_block_index + y * output_width + x] = false;
    }
  }
  snapshot_num_collapsed_cells = num_collapsed_cells;
}

size_t WFC::restartBlock() {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t block_size_x = settings.block_size_x;
  size_t block_size_y = settings.block_size_y;
  size_t current_block_index = current_block_y * output_width + current_block_x;
  for (size_t y = 0; y < block_size_y; y++) {
    for (size_t x = 0; x < block_size_x; x++) {
      size_t start_index_grid =
          (current_block_index + y * output_width + x) * num_64_blocks;
      size_t start_index_block = (y * block_size_x + x) * num_64_blocks;
      for (size_t k = 0; k < num_64_blocks; k++) {
        grid[start_index_grid + k] = backtracking_block[start_index_block + k];
      }
      is_cell_collapsed[current_block_index + y * output_width + x] = false;
    }
  }
  num_collapsed_cells = snapshot_num_collapsed_cells;
  return current_block_y * output_width + current_block_x;
}

size_t WFC::moveToNextBlock() {
  size_t block_size_x = settings.block_size_x;
  size_t block_size_y = settings.block_size_y;
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  // For convenience we copy the collapsed pixel into the backtracking block
  size_t current_block_index = current_block_y * output_width + current_block_x;
  for (size_t i = 0; i < num_64_blocks; i++) {
    backtracking_block[i] = grid[current_block_index * num_64_blocks + i];
  }

  // Now we copy back the block into the grid since we only keep the top left
  // corner of the block (or the leftmost column if we reached the bottom of the
  // image)
  size_t min_x = (current_block_y + block_size_y >= output_height) ? 1 : 0;
  size_t min_y = (current_block_x + block_size_x >= output_width) ? 1 : 0;
  for (size_t y = min_y; y < block_size_y; y++) {
    for (size_t x = min_x; x < block_size_x; x++) {
      size_t start_index_grid =
          (current_block_index + y * output_width + x) * num_64_blocks;
      size_t start_index_block = (y * block_size_x + x) * num_64_blocks;
      for (size_t k = 0; k < num_64_blocks; k++) {
        grid[start_index_grid + k] = backtracking_block[start_index_block + k];
      }
      is_cell_collapsed[current_block_index + y * output_width + x] = false;
      is_outside_block[current_block_index + y * output_width + x] = true;
    }
  }
  is_cell_collapsed[current_block_index] = true;

  // Update number of collapsed cells
  if (current_block_y + block_size_y >= output_height) {
    snapshot_num_collapsed_cells += block_size_y;
    num_collapsed_cells = snapshot_num_collapsed_cells;
  } else if (current_block_x + block_size_x >= output_width) {
    snapshot_num_collapsed_cells += block_size_x;
    num_collapsed_cells = snapshot_num_collapsed_cells;
  } else {
    snapshot_num_collapsed_cells++;
    num_collapsed_cells = snapshot_num_collapsed_cells;
  }

  // Now we update the block index
  if (current_block_x + block_size_x < output_width) {
    current_block_x++;
  } else {
    current_block_x = 0;
    current_block_y++;
  }

  // And finally we copy this part of the grid into the backtracking block
  updateBackTrackingBlock();

  return current_block_y * output_width + current_block_x;
}

size_t WFC::findCellToCollapse(size_t previous_cell_index) {
  size_t previous_x = previous_cell_index % output_width - current_block_x;

  if (previous_x + 1 < settings.block_size_x) {
    return previous_cell_index + 1;
  } else {
    size_t next_y = (previous_cell_index / output_width) + 1;
    if (next_y < output_height) {
      return next_y * output_width + current_block_x;
    } else {
      // Reached the end of the grid
      return SIZE_MAX;
    }
  }
}

void WFC::collapsePatternAtCell(size_t cell_index) {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;

  std::span<const pattern_id_t> possible_patterns =
      readPatternsAtCell(cell_index);

  // Find the frequency count of each pattern to create a discrete
  // distribution
  size_t total_counts = 0;
  pattern_frequencies.clear();
  for (pattern_id_t id : possible_patterns) {
    pattern_frequencies.push_back(adjacent_data.getPatternFrequency(id));
    total_counts += pattern_frequencies.back();
  }

  std::uniform_int_distribution<size_t> dist(0, total_counts - 1);
  size_t random_value = dist(rng);
  for (size_t i = 0; i < pattern_frequencies.size(); i++) {
    if (random_value < pattern_frequencies[i]) {
      // Found the selected pattern
      pattern_id_t selected_pattern_id = possible_patterns[i];
      for (size_t j = 0; j < num_64_blocks; j++) {
        grid[start_index + j] = 0;
      }

      size_t target_block = selected_pattern_id / 64;
      int target_bit = selected_pattern_id % 64;
      grid[start_index + target_block] = (1ULL << target_bit);
      num_collapsed_cells++;
      is_cell_collapsed[cell_index] = 1;
      collapsed_patterns[cell_index] = selected_pattern_id;
      std::cout << "Collapsed cell " << cell_index << " to pattern "
                << selected_pattern_id << std::endl;
      return;
    }
    random_value -= pattern_frequencies[i];
  }
}

std::span<const pattern_id_t> WFC::readPatternsAtCell(size_t cell_index) {
  if (is_cell_collapsed[cell_index]) {
    return {&collapsed_patterns[cell_index], 1};
  }

  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;

  // Find all the significant bits in the 64-bit blocks
  cell_pattern_ids.clear();
  for (size_t i = 0; i < num_64_blocks; i++) {
    uint64_t block = grid[start_index + i];
    if (block == 0)
      continue;

    pattern_id_t block_base_id = i * 64; // Accumulated id for each block
    while (block != 0) {
      pattern_id_t zeros_until_id = std::countr_zero(block);
      cell_pattern_ids.push_back(block_base_id + zeros_until_id);
      block &= (block - 1); // Clear the least significant bit set
    }
  }

  return {cell_pattern_ids.data(), cell_pattern_ids.size()};
}

uint8_t WFC::propagateConstraints(size_t cell_index,
                                  uint8_t is_global_propagation) {
  temp_buffers.current_version++;
  temp_buffers.current_cell_wave.clear();
  temp_buffers.current_cell_wave.push_back(cell_index);
  temp_buffers.cell_update_versions[cell_index] = temp_buffers.current_version;
  uint8_t no_contradictions = true;
  while (!temp_buffers.current_cell_wave.empty()) {
    no_contradictions = extendPropagationRange(is_global_propagation);
    if (!no_contradictions) {
      break; // Stop propagation if a contradiction is found
    }
    temp_buffers.current_cell_wave.swap(temp_buffers.next_cell_wave);
  }
  return no_contradictions;
}

uint8_t WFC::extendPropagationRange(uint8_t is_global_propagation) {
  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  temp_buffers.next_cell_wave.clear();
  for (size_t cell_index : temp_buffers.current_cell_wave) {
    // Restart version so it can be added back to the wave if needed
    temp_buffers.cell_update_versions[cell_index] = 0;
    std::span<const uint64_t> cell_constraints =
        getConstraintsFromCell(cell_index);
    // Check every direction
    for (size_t i = 0; i < NUM_DIRECTIONS_2D; i++) {
      size_t neighbour_index =
          neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i];
      if (neighbour_index == SIZE_MAX) {
        continue; // Out of bounds
      }
      if (!is_global_propagation && is_outside_block[neighbour_index]) {
        continue; // Skip propagation outside the block if not global
      }

      auto [has_changed, no_contradictions] = updateConstraintsOfNeighbour(
          {&cell_constraints[i * num_64_blocks], num_64_blocks},
          neighbour_index);
      if (!no_contradictions) {
        return false; // Stop propagation if a contradiction is found
      }
      if (has_changed && temp_buffers.cell_update_versions[neighbour_index] !=
                             temp_buffers.current_version) {
        temp_buffers.cell_update_versions[neighbour_index] =
            temp_buffers.current_version;
        temp_buffers.next_cell_wave.push_back(neighbour_index);
      }
    }
  }
  return true;
}

std::pair<uint8_t, uint8_t>
WFC::updateConstraintsOfNeighbour(std::span<const uint64_t> cell_constraints,
                                  size_t neighbour_index) {

  if (is_cell_collapsed[neighbour_index]) {
    return {false, true}; // No need to update a collapsed cell
  }

  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  uint8_t has_changed = false;
  uint8_t no_contradictions = false;
  size_t start_index = neighbour_index * num_64_blocks;
  for (size_t i = 0; i < num_64_blocks; i++) {
    uint64_t old_block = grid[start_index + i];
    uint64_t new_block = old_block & cell_constraints[i];
    if (new_block > 0) {
      no_contradictions = true;
    }
    if (new_block != old_block) {
      grid[start_index + i] = new_block;
      has_changed = true;
    }
  }

  return {has_changed, no_contradictions};
}

std::span<const uint64_t> WFC::getConstraintsFromCell(size_t cell_index) {
  if (is_cell_collapsed[cell_index]) {
    pattern_id_t collapsed_pattern_id = collapsed_patterns[cell_index];
    return adjacent_data.getConstraintsForPattern(collapsed_pattern_id);
  }

  size_t num_64_blocks = adjacent_data.getNum64Blocks();
  size_t start_index = cell_index * num_64_blocks;
  std::fill(temp_buffers.constraints_from_cell.begin(),
            temp_buffers.constraints_from_cell.end(), 0ULL);

  // Find all the significant bits in the 64-bit blocks
  for (size_t i = 0; i < num_64_blocks; i++) {
    uint64_t block = grid[start_index + i];
    if (block == 0)
      continue;

    pattern_id_t block_base_id = i * 64; // Accumulated id for each block
    while (block != 0) {
      pattern_id_t pattern_id = block_base_id + std::countr_zero(block);
      std::span<const uint64_t> constraints_from_pattern =
          adjacent_data.getConstraintsForPattern(pattern_id);

      for (size_t j = 0; j < NUM_DIRECTIONS_2D * num_64_blocks; j++) {
        temp_buffers.constraints_from_cell[j] |= constraints_from_pattern[j];
      }
      block &= (block - 1); // Clear the least significant bit set
    }
  }

  return {temp_buffers.constraints_from_cell.data(),
          temp_buffers.constraints_from_cell.size()};
}