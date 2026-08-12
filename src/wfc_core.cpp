#include "wfc_settings.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <directions.h>
#include <math.h>
#include <random>
#include <span>
#include <vector>
#include <wfc_core.h>
#include <wfc_globals.h>
#include <wfc_typedefs.h>

std::span<const pattern_id_t>
WFCCore::solve(size_t grid_width, size_t grid_height, size_t start_index,
               bool force_boundary_patterns, CellSelectionStrategy cell_selection_strategy,
               const std::unordered_map<size_t, pattern_id_t> &fixed_cells) {
  startSolver(grid_width, grid_height, force_boundary_patterns, fixed_cells);

  uint32_t num_restarts = 0;
  bool success = generateCollapsedGrid(start_index, cell_selection_strategy);
  while (!success && num_restarts < max_restarts) {
    startSolver(grid_width, grid_height, force_boundary_patterns, fixed_cells);
    success = generateCollapsedGrid(start_index, cell_selection_strategy);
    ++num_restarts;
  }
  if (!success) {
    return {};
  }

  return {collapsed_patterns.data(), collapsed_patterns.size()};
}

/* Manually collapse a given cell, if said cell is not collapsed already and if the selected pattern
 * is valid for that cell.*/
bool WFCCore::collapseSelectedCell(size_t cell_index, pattern_id_t pattern_id) {
  if (is_cell_collapsed[cell_index]) {
    return false;
  }
  size_t pattern_block = pattern_id >> 6;
  size_t pattern_bit = pattern_id & 63;
  if (!(grid[cell_index * num64_blocks + pattern_block] & (1ULL << pattern_bit))) {
    return false;
  }

  saveSnapshot(cell_index);
  collapsePatternAtCell(cell_index, pattern_id);

  bool no_contradictions = propagateConstraints(cell_index);
  if (!no_contradictions) {
    restoreSnapshot();
    propagateConstraints(cell_index);
  }

  return no_contradictions;
}

void WFCCore::undoLastCollapse() {
  if (stack_counter == 0) {
    return;
  }
  restoreSnapshot();
}

/* Return a grid of the desired dimensions where pixels that have been collapsed are marked as white
 * and pixels that have not been collapsed are marked as black. If the grid dimensions are smaller
 * than the output dimensions, the bottom and right edges of the output grid will be black.
 */
std::vector<uint8_t> WFCCore::getValidCellsForPattern(pattern_id_t pattern_id, size_t output_width,
                                                      size_t output_height) {
  size_t pattern_block = pattern_id >> 6;
  uint64_t pattern_mask = (1ULL << (pattern_id & 63));
  std::vector<uint8_t> valid_cells(output_width * output_height, 0);
  for (size_t block_index = 0; block_index < collapsed_mask.size(); ++block_index) {
    uint64_t collapsed_block = collapsed_mask[block_index];
    if (collapsed_block == 0ULL) {
      continue;
    }
    size_t base_cell_index = block_index << 6;
    while (collapsed_block != 0) {
      size_t cell_index = base_cell_index + std::countr_zero(collapsed_block);
      collapsed_block &= (collapsed_block - 1);
      if (grid[cell_index * num64_blocks + pattern_block] & pattern_mask) {
        size_t cell_x = cell_index % grid_width;
        size_t cell_y = cell_index / grid_width;
        valid_cells[cell_x + cell_y * output_width] = 255;
      }
    }
  }
  return valid_cells;
}

void WFCCore::startSolver(size_t grid_width, size_t grid_height, bool force_boundary_patterns,
                          const std::unordered_map<size_t, pattern_id_t> &fixed_cells) {
  this->grid_width = grid_width;
  this->grid_height = grid_height;
  total_cells = grid_width * grid_height;
  num64_blocks = adjacent_data.getNum64Blocks();
  scan_direction = 1;

  // Initialize grid and other variables
  initializeGrid(grid_width, grid_height);
  bakeNeighbourIndexes();
  initializeTempBuffers();
  num_collapsed_cells = 0;
  num_contradictions = 0;

  // Boundary conditions
  initializeEntropyData();
  if (force_boundary_patterns) {
    applyBoundaryConditions();
  }

  initializeEntropyData();
  initializeUndoStack();
  collapsedFixedCells(fixed_cells);
}

void WFCCore::collapsedFixedCells(const std::unordered_map<size_t, pattern_id_t> &fixed_cells) {
  for (const auto &[cell_index, pattern_id] : fixed_cells) {
    collapseSelectedCell(cell_index, pattern_id);
  }
}

bool WFCCore::generateCollapsedGrid(size_t start_index,
                                    CellSelectionStrategy cell_selection_strategy) {
  this->start_index = start_index;

  size_t current_cell_index;
  uint8_t no_contradictions;
  uint32_t iterations_per_snapshot = 1;
  uint32_t snapshot_iterator = 0;
  if (cell_selection_strategy == CellSelectionStrategy::ENTROPY) {
    current_cell_index = chooseNextCellEntropy();
  } else {
    current_cell_index = start_index;
  }
  saveSnapshot(current_cell_index);

  // Collapse cells until the whole grid is collapsed
  while (num_collapsed_cells < total_cells) {
    if (snapshot_iterator >= iterations_per_snapshot) {
      snapshot_iterator = 0;
      saveSnapshot(current_cell_index);
    }
    collapseRandomPatternAtCell(current_cell_index);
    no_contradictions = propagateConstraints(current_cell_index);
    if (!no_contradictions) {
      snapshot_iterator = 0;
      current_cell_index = restoreSnapshot();
      // propagateConstraints(current_cell_index);
      if (num_contradictions >= max_contradictions) {
        return false;
      }
      continue;
    }
    ++snapshot_iterator;
    if (cell_selection_strategy == CellSelectionStrategy::ENTROPY) {
      current_cell_index = chooseNextCellEntropy();
    } else {
      current_cell_index = chooseNextCellScanline(current_cell_index);
    }
  }

  return true;
}

void WFCCore::initializeGrid(size_t grid_width, size_t grid_height) {
  // First use 1's to mark every pattern as possible
  grid.assign(total_cells * num64_blocks, ~0ULL);

  // Clear the bits that are beyond the number of patterns
  size_t bits_to_remove = adjacent_data.getNumPatterns() & 63;
  uint64_t tail_mask = ~(0ULL);
  if (bits_to_remove > 0) {
    tail_mask = (1ULL << bits_to_remove) - 1;
  }

  // Remove exclusively boundary patterns from the grid
  std::span<const uint64_t> exclusively_boundary_patterns =
      adjacent_data.getExclusivelyBoundaryPatterns();

  for (size_t y = 0; y < grid_height; ++y) {
    for (size_t x = 0; x < grid_width; ++x) {
      size_t cell_index = y * grid_width + x;
      grid[cell_index * num64_blocks + num64_blocks - 1] &= tail_mask;
      if (y == 0 || y == grid_height - 1 || x == 0 || x == grid_width - 1) {
        continue; // Skip boundary cells
      }
      bool impossible_grid = true;
      for (size_t j = 0; j < num64_blocks; ++j) {
        grid[cell_index * num64_blocks + j] &= ~exclusively_boundary_patterns[j];
        impossible_grid &= grid[cell_index * num64_blocks + j] == 0;
      }
      assert(!impossible_grid && "Error: grid has no possible solution.");
    }
  }

  is_cell_collapsed.assign(total_cells, 0);
  collapsed_mask.assign((total_cells + 63) >> 6, ~0ULL);
  collapsed_patterns.assign(total_cells, 0);
  if (total_cells & 63) {
    collapsed_mask.back() = (1ULL << (total_cells & 63)) - 1;
  }
}

void WFCCore::initializeTempBuffers() {
  temp_buffers.cell_pattern_ids.reserve(num64_blocks * 64);
  temp_buffers.pattern_frequencies.reserve(num64_blocks * 64);
  temp_buffers.constraints_from_cell.resize(num64_blocks * NUM_DIRECTIONS_2D, 0);
  temp_buffers.cell_update_versions.resize(total_cells, 0);
  temp_buffers.next_cell_wave.resize(total_cells);
  temp_buffers.current_cell_wave.resize(total_cells);
  temp_buffers.current_version = 0;
}

void WFCCore::initializeEntropyData() {
  entropy_data.weight_times_log_weights.clear();
  entropy_data.weight_times_log_weights.resize(total_cells, 0);
  entropy_data.weight_sums.clear();
  entropy_data.weight_sums.resize(total_cells, 0);
  for (size_t i = 0; i < total_cells; ++i) {
    double weight_times_log_weight = 0;
    double weight_sum = 0;
    for (size_t j = 0; j < num64_blocks; ++j) {
      uint64_t pattern_block = grid[i * num64_blocks + j];
      size_t block_base_id = j << 6;
      while (pattern_block != 0) {
        pattern_id_t pattern_id = block_base_id + std::countr_zero(pattern_block);
        weight_times_log_weight += adjacent_data.getPatternFreqTimesLogFreq(pattern_id);
        weight_sum += adjacent_data.getPatternFrequency(pattern_id);
        pattern_block &= (pattern_block - 1);
      }
    }
    entropy_data.weight_times_log_weights[i] = weight_times_log_weight;
    entropy_data.weight_sums[i] = weight_sum;
  }
}

double WFCCore::calculateEntropyAtCell(size_t cell_index) {
  double w = entropy_data.weight_sums[cell_index];
  double w_log_w = entropy_data.weight_times_log_weights[cell_index];

  if (w == 0) {
    return 0; // No patterns available, entropy is zero
  }

  return std::log(w) - (w_log_w / w);
}

void WFCCore::initializeUndoStack() {
  undo_stack.clear();
  undo_stack.reserve((num64_blocks + 3) * total_cells / 100); // 1% of the whole grid
  stack_checkpoints.clear();
  stack_checkpoints.reserve(max_failed_snapshots);
  num_collapsed_cells_at_snapshot.clear();
  num_collapsed_cells_at_snapshot.reserve(1000);
  stack_starting_cell_indexes.clear();
  stack_starting_cell_indexes.reserve(1000);
  failures_at_snapshots.clear();
  failures_at_snapshots.reserve(1000);
  stack_counter = 0;
  current_snapshot = 0;
}

/* Bake the neighbour indexes for each cell to avoid recalculating them
 * during propagation */
void WFCCore::bakeNeighbourIndexes() {
  neighbour_indexes.resize(total_cells * NUM_DIRECTIONS_2D);
  for (size_t cell_index = 0; cell_index < total_cells; ++cell_index) {
    size_t x = cell_index % grid_width;
    size_t y = cell_index / grid_width;
    for (size_t i = 0; i < NUM_DIRECTIONS_2D; i++) {
      int dx = DX[i];
      int dy = DY[i];
      if (static_cast<int>(y) + dy < 0 || y + dy >= grid_height) {
        neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = SIZE_MAX;
        continue; // Out of bounds
      }
      if (static_cast<int>(x) + dx < 0 || x + dx >= grid_width) {
        neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = SIZE_MAX;
        continue; // Out of bounds
      }
      size_t neighbour_index = cell_index + dy * grid_width + dx;
      neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i] = neighbour_index;
    }
  }
}

/* Use only the patterns that are at the boundaries of the input image to restrict the patterns that
 * can be placed at the boundaries of the output image */
void WFCCore::applyBoundaryConditions() {
  // TOP and BOTTOM ROWS
  std::vector<size_t> y_indexes = {0, grid_height - 1};
  std::vector<size_t> directions = {Directions::UP, Directions::DOWN};
  for (size_t i = 0; i < 2; ++i) {
    size_t y = y_indexes[i];
    std::span<const uint64_t> boundary_patterns =
        adjacent_data.getPatternsAtBoundaries(directions[i]);
    for (size_t x = 0; x < grid_width; x++) {
      for (size_t j = 0; j < num64_blocks; j++) {
        grid[(y * grid_width + x) * num64_blocks + j] &= boundary_patterns[j];
      }
      propagateConstraints(y * grid_width + x);
    }
  }

  // LEFT and RIGHT COLUMNS
  std::vector<size_t> x_indexes = {0, grid_width - 1};
  directions = {Directions::LEFT, Directions::RIGHT};
  for (size_t i = 0; i < 2; ++i) {
    size_t x = x_indexes[i];
    std::span<const uint64_t> boundary_patterns =
        adjacent_data.getPatternsAtBoundaries(directions[i]);
    for (size_t y = 1; y < grid_height - 1; ++y) {
      for (size_t j = 0; j < num64_blocks; ++j) {
        grid[(y * grid_width + x) * num64_blocks + j] &= boundary_patterns[j];
      }
      propagateConstraints(y * grid_width + x);
    }
  }
}

size_t WFCCore::chooseNextCellScanline(size_t previous_cell_index) {
  size_t next_cell_index = previous_cell_index;
  while (is_cell_collapsed[next_cell_index]) {
    if (next_cell_index == total_cells - 1) {
      scan_direction = -1;               // Change direction to backward
      next_cell_index = start_index - 1; // Now go backwards from start_index - 1 to 0
    } else {
      next_cell_index += scan_direction;
    }
  }
  return next_cell_index;
}

size_t WFCCore::chooseNextCellEntropy() {
  size_t next_cell_index = SIZE_MAX;
  double min_entropy = std::numeric_limits<double>::max();
  for (size_t block_index = 0; block_index < collapsed_mask.size(); ++block_index) {
    uint64_t collapsed_block = collapsed_mask[block_index];
    if (collapsed_block == 0ULL) {
      continue; // Skip fully collapsed blocks
    }
    size_t base_cell_index = block_index << 6;
    while (collapsed_block != 0) {
      size_t cell_index = base_cell_index + std::countr_zero(collapsed_block);
      collapsed_block &= (collapsed_block - 1);
      double entropy = calculateEntropyAtCell(cell_index);

      if (entropy < min_entropy) {
        min_entropy = entropy;
        next_cell_index = cell_index;
      }
    }
  }
  return next_cell_index;
}

void WFCCore::collapseRandomPatternAtCell(size_t cell_index) {
  if (is_cell_collapsed[cell_index]) {
    return;
  }

  std::span<const pattern_id_t> possible_patterns = readPatternsAtCell(cell_index);
  pattern_id_t selected_pattern_id = possible_patterns[0];
  if (possible_patterns.size() > 1) {
    // Find the frequency count of each pattern to create a discrete
    // distribution
    size_t total_counts = 0;
    temp_buffers.pattern_frequencies.clear();
    for (pattern_id_t id : possible_patterns) {
      temp_buffers.pattern_frequencies.push_back(adjacent_data.getPatternFrequency(id));
      total_counts += temp_buffers.pattern_frequencies.back();
    }

    std::uniform_int_distribution<size_t> dist(0, total_counts - 1);
    size_t random_value = dist(rng);
    for (size_t i = 0; i < temp_buffers.pattern_frequencies.size(); i++) {
      if (random_value < temp_buffers.pattern_frequencies[i]) {
        selected_pattern_id = possible_patterns[i];
        break;
      }
      random_value -= temp_buffers.pattern_frequencies[i];
    }
  }

  collapsePatternAtCell(cell_index, selected_pattern_id);
}

void WFCCore::collapsePatternAtCell(size_t cell_index, pattern_id_t pattern_id) {
  if (is_cell_collapsed[cell_index]) {
    return;
  }
  pushCellToUndoStack(cell_index);

  size_t start_index = cell_index * num64_blocks;
  for (size_t j = 0; j < num64_blocks; j++) {
    grid[start_index + j] = 0;
  }

  size_t pattern_block = pattern_id >> 6;
  size_t pattern_bit = pattern_id & 63;
  grid[start_index + pattern_block] = (1ULL << pattern_bit);
  num_collapsed_cells++;
  is_cell_collapsed[cell_index] = 1;
  collapsed_patterns[cell_index] = pattern_id;
  collapsed_mask[cell_index >> 6] &= ~(1ULL << (cell_index & 63));
}

std::span<const pattern_id_t> WFCCore::readPatternsAtCell(size_t cell_index) {
  if (is_cell_collapsed[cell_index]) {
    return {&collapsed_patterns[cell_index], 1};
  }

  size_t start_index = cell_index * num64_blocks;

  // Find all the significant bits in the 64-bit blocks
  temp_buffers.cell_pattern_ids.clear();
  for (size_t i = 0; i < num64_blocks; i++) {
    uint64_t block = grid[start_index + i];
    if (block == 0)
      continue;

    pattern_id_t block_base_id = i << 6;
    while (block != 0) {
      pattern_id_t pattern_id = std::countr_zero(block);
      temp_buffers.cell_pattern_ids.push_back(block_base_id + pattern_id);
      block &= (block - 1); // Clear the least significant bit set
    }
  }

  return {temp_buffers.cell_pattern_ids.data(), temp_buffers.cell_pattern_ids.size()};
}

uint8_t WFCCore::propagateConstraints(size_t cell_index) {
  temp_buffers.current_version++;
  temp_buffers.current_cell_wave.clear();
  temp_buffers.current_cell_wave.push_back(cell_index);
  temp_buffers.cell_update_versions[cell_index] = temp_buffers.current_version;
  uint8_t no_contradictions = true;
  while (!temp_buffers.current_cell_wave.empty()) {
    no_contradictions = extendPropagationRange();
    if (!no_contradictions) {
      break; // Stop propagation if a contradiction is found
    }
    temp_buffers.current_cell_wave.swap(temp_buffers.next_cell_wave);
  }
  return no_contradictions;
}

uint8_t WFCCore::extendPropagationRange() {
  temp_buffers.next_cell_wave.clear();
  for (size_t cell_index : temp_buffers.current_cell_wave) {
    // Restart version so it can be added back to the wave if needed
    temp_buffers.cell_update_versions[cell_index] = 0;
    std::span<const uint64_t> cell_constraints = getConstraintsFromCell(cell_index);
    // Check every direction
    for (size_t i = 0; i < NUM_DIRECTIONS_2D; ++i) {
      size_t neighbour_index = neighbour_indexes[cell_index * NUM_DIRECTIONS_2D + i];
      if (neighbour_index == SIZE_MAX) {
        continue; // Out of bounds
      }

      auto [has_changed, no_contradictions] = updateConstraintsOfNeighbour(
          {&cell_constraints[i * num64_blocks], num64_blocks}, neighbour_index);
      if (!no_contradictions) {
        return false; // Stop propagation if a contradiction is found
      }
      if (has_changed &&
          temp_buffers.cell_update_versions[neighbour_index] != temp_buffers.current_version) {
        temp_buffers.cell_update_versions[neighbour_index] = temp_buffers.current_version;
        temp_buffers.next_cell_wave.push_back(neighbour_index);
      }
    }
  }
  return true;
}

std::pair<uint8_t, uint8_t>
WFCCore::updateConstraintsOfNeighbour(std::span<const uint64_t> cell_constraints,
                                      size_t neighbour_index) {
  if (is_cell_collapsed[neighbour_index]) {
    return {false, true}; // No need to update a collapsed cell
  }
  uint8_t has_changed = false;
  uint8_t already_pushed_to_stack = false;
  uint64_t no_contradictions = false;
  size_t start_index = neighbour_index * num64_blocks;
  for (size_t i = 0; i < num64_blocks; ++i) {
    uint64_t old_block = grid[start_index + i];
    uint64_t new_block = old_block & cell_constraints[i];
    no_contradictions |= new_block;
    if (new_block != old_block) {
      has_changed = true;
      if (!already_pushed_to_stack) {
        pushCellToUndoStack(neighbour_index);
        already_pushed_to_stack = true;
      }
      grid[start_index + i] = new_block;
      uint64_t removed_patterns = old_block & ~new_block;
      while (removed_patterns != 0) {
        pattern_id_t removed_pattern_id = (i << 6) + std::countr_zero(removed_patterns);
        entropy_data.weight_times_log_weights[neighbour_index] -=
            adjacent_data.getPatternFreqTimesLogFreq(removed_pattern_id);
        entropy_data.weight_sums[neighbour_index] -=
            adjacent_data.getPatternFrequency(removed_pattern_id);
        removed_patterns &= (removed_patterns - 1);
      }
    }
  }

  return {has_changed, no_contradictions != 0};
}

/* Retrieve the constraints a cell applies to its neighbours */
std::span<const uint64_t> WFCCore::getConstraintsFromCell(size_t cell_index) {
  if (is_cell_collapsed[cell_index]) {
    return adjacent_data.getConstraintsFromPattern(collapsed_patterns[cell_index]);
  }

  size_t start_index = cell_index * num64_blocks;
  std::fill(temp_buffers.constraints_from_cell.begin(), temp_buffers.constraints_from_cell.end(),
            0ULL);

  size_t total_blocks = NUM_DIRECTIONS_2D * num64_blocks;
  // Find all the significant bits in the 64-bit blocks
  for (size_t i = 0; i < num64_blocks; ++i) {
    uint64_t block = grid[start_index + i];
    pattern_id_t block_base_id = i << 6; // Accumulated id for each block
    while (block != 0) {
      std::span<const uint64_t> constraints_from_pattern =
          adjacent_data.getConstraintsFromPattern(block_base_id + std::countr_zero(block));

      for (size_t j = 0; j < total_blocks; ++j) {
        temp_buffers.constraints_from_cell[j] |= constraints_from_pattern[j];
      }
      block &= (block - 1);
    }
  }

  return {temp_buffers.constraints_from_cell.data(), temp_buffers.constraints_from_cell.size()};
}

void WFCCore::saveSnapshot(size_t current_cell_index) {
  failed_snapshots = 0;
  stack_checkpoints.push_back(stack_counter);
  num_collapsed_cells_at_snapshot.push_back(num_collapsed_cells);
  stack_starting_cell_indexes.push_back(current_cell_index);
  failures_at_snapshots.push_back(0);
  if (num_contradictions > 0) {
    --num_contradictions;
  }
  ++current_snapshot;
}

size_t WFCCore::restoreSnapshot() {
  stack_counter = stack_checkpoints.back();
  size_t cell_index = 0;
  size_t stack_size = num64_blocks + 3;
  size_t num_updates = undo_stack.size() / stack_size - stack_counter;
  for (size_t i = num_updates; i > 0; --i) {
    size_t start = stack_size * (i - 1 + stack_counter);
    cell_index = static_cast<size_t>(undo_stack[start]);
    if (cell_index == total_cells - 1) {
      scan_direction = 1; // Reset scan direction to forward
    }

    entropy_data.weight_sums[cell_index] = std::bit_cast<double>(undo_stack[start + 1]);
    entropy_data.weight_times_log_weights[cell_index] =
        std::bit_cast<double>(undo_stack[start + 2]);

    for (size_t j = 0; j < num64_blocks; ++j) {
      grid[cell_index * num64_blocks + j] = undo_stack[start + 3 + j];
    }
    is_cell_collapsed[cell_index] = false;
    collapsed_mask[cell_index >> 6] |= (1ULL << (cell_index & 63));
  }
  // Remove the patterns that were collapsed in the snapshot
  cell_index = stack_starting_cell_indexes.back();
  pattern_id_t collapsed_pattern_id = collapsed_patterns[cell_index];
  size_t target_block = collapsed_pattern_id / 64;
  int target_bit = collapsed_pattern_id % 64;
  grid[cell_index * num64_blocks + target_block] &= ~(1ULL << target_bit);

  undo_stack.resize(stack_counter * stack_size);
  num_collapsed_cells = num_collapsed_cells_at_snapshot.back();
  ++failures_at_snapshots.back();
  ++num_contradictions;
  if (failures_at_snapshots.back() >= max_failures_per_snapshot) {
    return goToPreviousSnapshot();
  }

  return cell_index;
}

size_t WFCCore::goToPreviousSnapshot() {
  ++failed_snapshots;
  --current_snapshot;
  if (current_snapshot == 0 || failed_snapshots >= max_failed_snapshots) {
    num_contradictions = max_contradictions; // Force restart
    return SIZE_MAX;
  }

  failures_at_snapshots.pop_back();
  stack_checkpoints.pop_back();
  num_collapsed_cells_at_snapshot.pop_back();
  stack_starting_cell_indexes.pop_back();
  // If last snapshot was a total failure, add one failure to the previous snapshot
  if (!failures_at_snapshots.empty()) {
    ++failures_at_snapshots.back();
  }
  return restoreSnapshot();
}

void WFCCore::pushCellToUndoStack(size_t cell_idx) {
  // Order of data starting from end:
  // [Index][WeightSum][WLogW][Block0][Block1]...
  undo_stack.push_back(static_cast<uint64_t>(cell_idx));
  undo_stack.push_back(std::bit_cast<uint64_t>(entropy_data.weight_sums[cell_idx]));
  undo_stack.push_back(std::bit_cast<uint64_t>(entropy_data.weight_times_log_weights[cell_idx]));

  size_t start = cell_idx * num64_blocks;
  for (size_t i = 0; i < num64_blocks; ++i) {
    undo_stack.push_back(grid[start + i]);
  }
  ++stack_counter;
}