#pragma once

#include <math.h>
#include <vector>

struct EntropyData {
  std::vector<double> weight_times_log_weights;
  std::vector<double> weight_sums;

  double getEntropy(size_t cell_index) const {
    double weight_times_log_weight = weight_times_log_weights[cell_index];
    double weight_sum = weight_sums[cell_index];
    if (weight_sum == 0) {
      return 0; // Avoid division by zero
    }
    return std::log(weight_sum) - (weight_times_log_weight / weight_sum);
  }
};