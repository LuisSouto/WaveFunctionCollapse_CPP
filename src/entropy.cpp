#include <cmath>
#include <entropy.h>

double shannonEntropy(const std::vector<uint64_t> &frequencies) {
  double total = 0.0;
  for (uint64_t freq : frequencies) {
    total += freq;
  }
  double entropy = 0.0;
  for (uint64_t freq : frequencies) {
    double p = static_cast<double>(freq) / total;
    entropy -= p * std::log2(p);
  }
  return entropy;
}