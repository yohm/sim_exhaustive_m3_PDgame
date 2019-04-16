#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <utility>
#include <deque>
#include "mpi.h"
#include "Strategy.hpp"

using namespace std;

#ifdef NDEBUG
#define DP(x) do {} while (0)
#else
#define DP(x) do { std::cerr << x << std::endl; } while (0)
#endif

std::vector<State> UndeterminedNegativeNode(const Strategy& s) {
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

int64_t n_determined = 0;
int64_t n_pending = 0;
int64_t n_rejected = 0;
int n_target_fixed = 64;

void ExploreNegativeDangling(const Strategy& s, const int depth, vector<Strategy>& found) {
  if(depth == 0 || s.NumFixed() >= n_target_fixed) {
    if(depth==0) { DP("reached maximum depth : " << s.NegativeDanglingStates().size()); }
    else { DP("reached targeted number of fixed actions"); }
    n_pending++;
    found.push_back(s);
    return;
  }

  vector<State> negs = s.NegativeDanglingStates();
  vector<State> undetermined = UndeterminedNegativeNode(s);
  /*
  for(auto s: negs) {
    std::cerr << s << std::endl;
  }
  std::cerr << std::endl;
  for(auto s: undetermined) {
    std::cerr << s << std::endl;
  }
   */
  if(negs.size() > 0 || undetermined.size() > 0) {
    if(negs.size() > 0) { DP("negative dangling node is found: " << negs[0]); }
    else { DP("negative undetermined node is found: " << undetermined[0]); }

    // if(negs.size() + undetermined.size() < 16)  // heuristic
    { // check defensibility of wildcard
      Strategy _s = ReplaceUwithW(s);
      if(_s.IsDefensible()) {
        DP("WILDCARD worked! at depth " << depth << " : " << negs.size()+undetermined.size());
        found.push_back(_s);
        n_determined++;
        return;
      }
    }

    const State target = (negs.size()>0) ? negs[0] : undetermined[0];
    /*
    if( negs.size() >= depth ) {
      DP("too many negative dangling states. Impossible to judge : " << negs.size() << " " << depth );
      n_pending++;
      found.push_back(s);
      return;
    }
     */
    for(int i=0; i<2; i++) {
      Strategy _s = s;
      Action _a = (i==0 ? C : D);
      bool defensible = _s.SetActionAndRecalcD(target, _a);
      if(!defensible) {
        DP("not defensible.");
        n_rejected++;
        continue;
      }
      ExploreNegativeDangling(_s, depth-1, found);
    }
  }
  else { // no negative undetermined node && no negative dangling node. It must be defensible.
    DP("No negative dangling node and undetermined node. must be defensible");
    Strategy _s = ReplaceUwithW(s);
    if( ! _s.IsDefensible() ) { throw "must not happen"; }
    found.push_back(_s);
    n_determined++;
  }
}

vector<Strategy> TraceNegativeDefensible(Strategy str, int max_depth) {
  vector<Strategy> found;
  if( ! str.IsDefensible() ) { // must be called since d_matrix must be prepared
    throw "must not happen";
  }
  if(str.NumU() == 0) {
    found.push_back(str);
  }
  else {
    ExploreNegativeDangling(str, max_depth, found);
  }
  return std::move(found);
}

void test() {
  // Strategy s1("ccddcccd____ccccdcdddd_d___d___d___d___c_d_c_c_d_______c_______d");
  // Strategy s1("c______d______dd_______d____dddd_______d______dd_______ddddddddd");
  // Strategy s1("c______________________________________________________________d");
  // Strategy s1("cd_____dd__d_ddd__c_d__d_c_cdddd__d_c__d_d_c_ccd_d_c_cdd_dcdcd_d");
  // Strategy s1("cdddcccdc_ccdc_c_c_dc__d_c_____dc_c____c_______d___d___c_______d");
  // Strategy s1("cdddcccdc_ccdcdc_cddcddddc__ddddcdcd_cdcdcdddcdd_cdddd_cdcddddcd");
  // Strategy s1("cd_____dc_cd_ddd_cc_d__d___cdddd__d_c__d_d___c_d______dd_d_d_d_d");
  Strategy s1("cdddcccdc_cd_cdc_c_dcdcd____ddddc_c_c__c__d___dd_______c______dd");
  // Strategy s1("cd**cdcdd*cd**cd****cdcc*c****cdcd**cdcc**cd***c***ccd*d******cd");
  auto found = TraceNegativeDefensible(s1, 30);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  cout << found.size() << " : " << n_determined << " / " << n_pending << " / " << n_rejected << endl;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#endif

  if( argc != 3 && argc != 4 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <max_depth> [target num fixed actions]" << endl;
    return 1;
  }

  if( argc == 4 ) {
    n_target_fixed = atoi(argv[3]);
  }

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; ) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << std::endl;
      std::cerr << "determined/pending/rejected :" << n_determined << " / " << n_pending << " / " << n_rejected << std::endl;
    }
    Strategy str(s.c_str());
    if(str.ActionAt("cccccc") == U) { str.SetAction("cccccc", C); }
    assert(str.ActionAt("cccccc") == C);
    auto found = TraceNegativeDefensible(str, atoi(argv[2]));
    for(auto s: found) {
      cout << s.ToString() << endl;
    }
    count++;
  }
  std::cerr << "determined/pending/rejected :" << n_determined << " / " << n_pending << " / " << n_rejected << std::endl;
  return 0;
}

