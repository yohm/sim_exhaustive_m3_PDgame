#ifndef CHECK_EFFICIENCY_BY_GRAPH_HPP
#define CHECK_EFFICIENCY_BY_GRAPH_HPP

#include <iostream>
#include "Strategy.hpp"
#include "MyLib.hpp"

class CheckEfficiencyResult_t {
public:
  std::vector<std::string> efficient_and_defensible;
  std::vector<Strategy> efficient;  // efficient, but defensibility is not assured
  uint64_t n_efficient_and_defensible;
  uint64_t n_rejected;
  CheckEfficiencyResult_t() : n_efficient_and_defensible(0), n_rejected(0) {};
  uint64_t NumEfficient() const {
    uint64_t n = 0;
    for(const Strategy& s: efficient) { n += s.Size(); }
    return n;
  }
  uint64_t NumTotal() const {
    return n_efficient_and_defensible + NumEfficient() + n_rejected;
  }
  void PrintStrategies(std::ostream &os) const {
    for(const Strategy& s: efficient) {
      os << "E: " << s.ToString() << std::endl;
    }
    os << "# E&D/E/P/R : " << ToC(n_efficient_and_defensible) << " / " << ToC(NumEfficient()) << " / " << ToC(n_rejected) << std::endl;
  }
};

CheckEfficiencyResult_t CheckEfficiencyByGraph(Strategy &str);

#endif
