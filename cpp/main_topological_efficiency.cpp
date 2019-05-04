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

// defensibilityが確定している戦略に対して、グラフのトポロジーから efficiency が確定できるか判定する
// - ITGを構築する
//   - 未確定ビット(*)が存在する場合は、{c,d}両方の場合のリンクを作成する
//   - ここで構築したグラフのcycleは、すべてのビットが確定した後のcycleをかならず含む。
// - ITGからcycleを検出する
//   - ここではSCCを検出し、そのSCCの中でself-loopが存在するかどうかでcycleを検出することを試みる
// - すべてのcycleが確定ビットのみで構成される場合
//   - すべてのcycleに対して「cycleの1-bit neighborのどれかから、State(0)に必ず到達するようなパスが存在する」が成り立つならば、efficiencyが確定する
// - cycleの中に未確定ビットが含まれている場合、それらのビットを確定して、再帰的に判定する。

#ifdef NDEBUG
#define DP(x) do {} while (0)
#else
#define DP(x) do { std::cerr << x << std::endl; } while (0)
#endif

void AssignActions(Strategy s, std::set<long> to_be_fixed, std::vector<Strategy>& ans) {
  if( to_be_fixed.empty() ) {
    ans.push_back(s);
    return;
  }

  auto it = to_be_fixed.begin();
  long n = *it;
  to_be_fixed.erase(it);
  s.SetAction(State(n), C);
  AssignActions(s, to_be_fixed, ans);
  s.SetAction(State(n), D);
  AssignActions(s, to_be_fixed, ans);
}

bool SurelyReach0(const DirectedGraph& g, long ini) {
  long current = ini;
  std::set<long> histo;
  while( histo.find(current) == histo.end() ) {
    histo.insert(current);
    if( g.m_links[current].size() != 1 ) { // cannot uniquely determine the destination
      return false;
    }
    current = g.m_links[current][0];
  }
  return (current == 0);
}

bool IsSurelyEfficient(const Strategy& str) {
  DirectedGraph g = str.ITG();
  components_t comps = g.NonTransitionComponents();
  for(const auto& comp: comps) {
    if( comp.size() == 1 && comp[0] == 0 ) { continue; }  // skip 'cccccc' component
    std::vector<long> neighbors;
    for(unsigned long n: comp) {
      neighbors.push_back(n^1UL);  // 1-bit neighbors
      neighbors.push_back(n^8UL);
    }
    bool return_to_0 = false;
    for(long neigh: neighbors) {
      if( SurelyReach0(g, neigh) ) {
        return_to_0 = true;
        break;
      }
    }
    if( !return_to_0 ) {
      return false;
    }
  }
  return true;
}

void CheckTopologicalEfficiency(const Strategy& str, std::vector<Strategy>& efficients, std::vector<Strategy>& unjudgeables) {
  DirectedGraph g = str.ITG();
  components_t comps = g.NonTransitionComponents();
  std::set<long> to_be_fixed;
  for(const auto& comp: comps) {
    for(long n: comp) {
      State sa(n);
      State sb = sa.SwapAB();
      Action act_a = str.ActionAt(sa);
      Action act_b = str.ActionAt(sb);
      if( act_a == U || act_a == W ) { to_be_fixed.insert(sa.ID()); }
      if( act_b == U || act_b == W ) { to_be_fixed.insert(sb.ID()); }
    }
  }

  assert(to_be_fixed.size() < 16);
  std::vector<Strategy> assigned;
  AssignActions(str, to_be_fixed, assigned);

  for(const Strategy& s: assigned) {
    if( IsSurelyEfficient(s) ) {
      efficients.push_back(s);
    }
    else {
      unjudgeables.push_back(s);
    }
  }
}

void test() {
  Strategy s("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd"); // is efficient
  std::vector<Strategy> efficients, unjudgeables;
  CheckTopologicalEfficiency(s, efficients, unjudgeables);
  for(auto s: efficients) {
    cout << "E: " << s.ToString() << endl;
  }
  for(auto s: unjudgeables) {
    cout << "U: " << s.ToString() << endl;
  }
}

Strategy ReplaceWwithU(const Strategy& s) {
  Strategy _s = s;
  for(int i=0; i<64; i++) { if( _s.actions[i] == W ) { _s.actions[i] = U; } }
  return std::move(_s);
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <max_depth>" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; ) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << std::endl;
      std::cerr << "recovered/pending/indefensible/rejected :" << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;
    }
    Strategy _str(s.c_str());
    Strategy str = ReplaceWwithU(_str);

    assert(str.ActionAt("cccccc") == C);
    auto found = SelectEfficientDefensible(str, atoi(argv[2]));
    for(auto s: found) {
      cout << s.ToString() << endl;
    }
    count++;
  }
  std::cerr << "recovered/pending/indefensible/rejected :" << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;
  return 0;
#endif
}

