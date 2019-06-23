#ifndef TOPOLOGICAL_EFFICIENCY_HPP
#define TOPOLOGICAL_EFFICIENCY_HPP

#include <iostream>
#include "Strategy.hpp"
#include "MyLib.hpp"

class TopologicalEfficiencyResult_t {
public:
  std::vector<std::string> efficient_and_defensible;
  std::vector<Strategy> efficient;
  std::vector<Strategy> pending;
  uint64_t n_efficient_and_defensible;
  uint64_t n_rejected;
  TopologicalEfficiencyResult_t() : n_efficient_and_defensible(0), n_rejected(0) {};
  uint64_t NumEfficient() const {
    uint64_t n = 0;
    for(const Strategy& s: efficient) { n += s.Size(); }
    return n;
  }
  uint64_t NumPending() const {
    uint64_t n = 0;
    for(const Strategy& s: pending) { n += s.Size(); }
    return n;
  }
  void PrintStrategies(std::ostream &os) const {
    for(const Strategy& s: efficient) {
      // os << "E: " << s.ToString() << std::endl;
    }
    for(const Strategy& s: pending) {
      // os << "P: " << s.ToString() << std::endl;
    }
    os << "# E&D/E/P/R : " << ToC(n_efficient_and_defensible) << " / " << ToC(NumEfficient()) << " / " << ToC(NumPending()) << " / " << ToC(n_rejected) << std::endl;
  }
};

TopologicalEfficiencyResult_t CheckTopologicalEfficiency(Strategy& str);

#endif
