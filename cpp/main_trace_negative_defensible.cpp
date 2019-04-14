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

int64_t n_determined = 0;
int64_t n_pending = 0;
int64_t n_rejected = 0;

void ExploreNegativeDangling(const Strategy& s, const int depth, vector<Strategy>& found) {
  if(depth == 0) {
    DP("reached maximum depth : " << s.NegativeDanglingStates().size());
    n_pending++;
    found.push_back(s);
    return;
  }

  vector<State> negs = s.NegativeDanglingStates();
  vector<State> undetermined = UndeterminedNegativeNode(s);
  if(negs.size() > 0 || undetermined.size() > 0) {
    if(negs.size() > 0) { DP("negative dangling node is found: " << negs[0]); }
    else { DP("negative undtermined node is found: " << undetermined[0]); }

    const State target = (negs.size()>0) ? negs[0] : undetermined[0];
    if( negs.size() >= depth ) {
      DP("too many negative dangling states. Impossible to judge : " << negs.size() << " " << depth );
      n_pending++;
      found.push_back(s);
      return;
    }
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
    DP("No negative dangling node and undtermined node. must be defensible");
    Strategy _s = s;
    for(int i=0; i<64; i++) { if( _s.actions[i] == U ) { _s.actions[i] = W; } }
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
  ExploreNegativeDangling(str, max_depth, found);
  return std::move(found);
}

void test() {
  // Strategy s1("ccddcccd____ccccdcdddd_d___d___d___d___c_d_c_c_d_______c_______d");
  // Strategy s1("c______d______dd_______d____dddd_______d______dd_______ddddddddd");
  // Strategy s1("c______________________________________________________________d");
  // Strategy s1("cd_____dd__d_ddd__c_d__d_c_cdddd__d_c__d_d_c_ccd_d_c_cdd_dcdcd_d");
  // Strategy s1("cdddcccdc_ccdc_c_c_dc__d_c_____dc_c____c_______d___d___c_______d");
  // Strategy s1("cdddcccdc_ccdcdc_cddcddddc__ddddcdcd_cdcdcdddcdd_cdddd_cdcddddcd");
  Strategy s1("cd_____dc_cd_ddd_cc_d__d___cdddd__d_c__d_d___c_d______dd_d_d_d_d");
  auto found = TraceNegativeDefensible(s1, 25);
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

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <max_depth>" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; ) {
    if(count % 100 == 0) {
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

