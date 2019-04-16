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

void ExploreEfficiency(const Strategy& s, vector<Strategy>& found) {
  if( s.NumFixed() == 64 ) {
    if( s.IsEfficient() ) { found.push_back(s); n_efficient++; }
    else { n_inefficient++; }
    return;
  }
  else {
    for(int i=0; i<64; i++) {
      if(s.ActionAt(i) == W || s.ActionAt(i) == U) {
        for(int j=0; j<2; j++) {
          Strategy _s = s;
          _s.SetAction(i, ((j==0)?C:D) );
          ExploreEfficiency(_s, found);
        }
        return;
      }
    }
  }
}

vector<Strategy> CheckEfficiency(const Strategy& str) {
  vector<Strategy> found;
  assert( str.NumU() == 0 );
  ExploreEfficiency(str, found);
  return std::move(found);
}

void test() {
  Strategy s1("cdddcccdc*cdccdccccdddddcc**ccddc*ddcd*cdccddcddddcd***c****ccdd");
  // Strategy s1("cd*cdd*dcdcd**cd*c*dcd*d**ccdcddddcdcd*d**ddddcd**dd**cd**dcdddd");
  auto found = CheckEfficiency(s1);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  cout << "efficient/inefficient : " << n_efficient << " / " << n_inefficient << endl;
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

