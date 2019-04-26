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

int n_efficient = 0;
int n_inefficient = 0;

struct Counts {
  int64_t n_efficient;
  int64_t n_inefficient;
  Counts() {
    n_efficient = 0;
    n_inefficient = 0;
  }
};

void ExploreEfficiency(const Strategy& s, Counts& counter) {
  assert( s.NumU() == 0 );
  if( s.NumFixed() == 64 ) {
    if( s.IsEfficient() ) { counter.n_efficient++;}
    else { counter.n_inefficient++; }
    return;
  }
  else {
    int idx = 0;
    for(; idx<64; idx++) {
      if(s.ActionAt(idx) == W || s.ActionAt(idx) == U) { break; }
    }
    for(int j=0; j<2; j++) {
      Strategy _s = s;
      _s.SetAction(idx, ((j==0)?C:D) );
      ExploreEfficiency(_s, counter);
    }
  }
}

Counts CheckEfficiency(const Strategy& str) {
  assert( str.NumU() == 0 );
  Counts counter;
  ExploreEfficiency(str, counter);
  return std::move(counter);
}

void test() {
  // Strategy s1("cdddcccdc*cdccdccccdddddcc**ccddc*ddcd*cdccddcddddcd***c****ccdd");
  Strategy s1("ccdd*c*dc*ccddcdcd*cddccdcdd**cd*d*dccddddcd******ccdd******cccd");  // partly efficient
  // Strategy s1("cd*dcd*dd*dd**dcddcd*ddd*ddddcdd*d*dcdcddddcccdd**dd***ccc*cdcdd");
  // Strategy s1("cd*d*d*dd*ddccdcddcdddcd*ddddcdd****cdddcddd**cd**cd***cdc*cdcdd"); // all efficient
  auto counter = CheckEfficiency(s1);
  cout << "efficient/inefficient : " << counter.n_efficient << " / " << counter.n_inefficient << endl;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

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
#endif
}

