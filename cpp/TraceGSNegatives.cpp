#include "TraceGSNegatives.hpp"
#include <iostream>
#include <vector>
#include "Strategy.hpp"

namespace {

  typedef TraceGSNegativesResult_t res_t;

  std::vector<State> UndeterminedNegativeNodes(const Strategy &s) {
    std::vector<State> ret;
    for(uint64_t i=0; i<64; i++) {
      if( (i&9)==1 ) { // "**c**d"
        State si(i);
        assert(si.RelativePayoff() < 0);
        if(s.ActionAt(si) == U) {
          ret.push_back(si);
        }
      }
      else {
        assert(State(i).RelativePayoff() >= 0);
      }
    }
    return std::move(ret);
  }

  Strategy ReplaceUwithW(const Strategy& s) {
    Strategy _s = s;
    for(int i=0; i<64; i++) { if( _s.actions[i] == U ) { _s.actions[i] = W; } }
    return std::move(_s);
  }

  void ExploreNegativeDangling(const Strategy& s, const int depth, const int target_fixed, res_t& res) {
    if(depth == 0 || s.NumFixed() >= target_fixed) {
      if(depth==0) { DP("reached maximum depth : " << s.NegativeDanglingStates().size()); }
      else { DP("reached targeted number of fixed actions"); }
      res.passed.push_back(s);
      return;
    }

    std::vector<State> negs = s.NegativeDanglingStates();
    std::vector<State> undetermined = UndeterminedNegativeNodes(s);

    if(negs.size() > 0 || undetermined.size() > 0) {
      if(negs.size() > 0) { DP("negative dangling node is found: " << negs[0]); }
      else { DP("negative undetermined node is found: " << undetermined[0]); }

      const State target = (negs.size()>0) ? negs[0] : undetermined[0];
      for(int i=0; i<2; i++) {
        Strategy _s = s;
        Action _a = (i==0 ? C : D);
        bool defensible = _s.SetActionAndRecalcD(target, _a);
        if(!defensible) {
          DP("not defensible.");
          res.n_rejected += _s.Size();
          res.n_rejection_events++;
          continue;
        }
        ExploreNegativeDangling(_s, depth-1, target_fixed, res);
      }
    }
    else { // no negative undetermined node && no negative dangling node. It must be defensible.
      DP("No negative dangling node and undetermined node. must be defensible");
      Strategy _s = ReplaceUwithW(s);
      if(!_s.IsDefensible()) { throw std::runtime_error("must not happen"); }
      res.passed.push_back(_s);
    }
  }
}

// check defensibility by tracing g(S,*) from negative dangling nodes.
TraceGSNegativesResult_t TraceGSNegatives(Strategy str, int max_depth, int target_fixed) {
  res_t res;
  if( ! str.IsDefensible() ) { // must be called since d_matrix must be prepared
    throw std::runtime_error("must not happen");
  }
  if(str.NumU() == 0) {  // it is sure that the strategy is defensible
    res.passed.push_back(str);
  }
  else {
    ExploreNegativeDangling(str, max_depth, target_fixed, res);
  }
  return std::move(res);
}

