#ifndef TRACE_NEGATIVE_DEFENSIBLE_HPP
#define TRACE_NEGATIVE_DEFENSIBLE_HPP

#include <iostream>
#include <vector>
#include "Strategy.hpp"
#include "MyLib.hpp"

class TraceNegativeDefensibleResult_t {
public:
  std::vector<Strategy> passed;
  uint64_t n_rejected;        // number of strategies
  uint64_t n_rejection_events;  // number of rejection events
  TraceNegativeDefensibleResult_t() : n_rejected(0ULL), n_rejection_events(0ULL) {};

  uint64_t NumPassed() const {
    uint64_t ans = 0;
    for(const auto& s: passed) { ans += s.Size(); }
    return ans;
  }
  uint64_t NumDefensible() const {
    uint64_t ans = 0;
    for(const auto& s: passed) {
      if( s.NumU() == 0 ) { ans += s.Size(); }
    }
    return ans;
  }
  std::vector<Strategy> DefensibleStrategies() const {
    std::vector<Strategy> ans;
    for(const auto& s: passed) {
      if(s.NumU() == 0) { ans.push_back(s); }
    }
    return std::move(ans);
  }
};

// Judge defensibility of the strategies by tracing negative dangling nodes.
TraceNegativeDefensibleResult_t TraceNegativeDefensible(Strategy str, int max_depth, int target_fixed);

#endif
