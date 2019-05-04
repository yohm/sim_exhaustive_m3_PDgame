#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <utility>
#include <deque>
#include <iomanip>
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

void AssignActions(const Strategy& s, std::set<long> to_be_fixed, std::vector<Strategy>& ans) {
  if( to_be_fixed.empty() ) {
    ans.push_back(s);
    return;
  }

  auto it = to_be_fixed.begin();
  long n = *it;
  to_be_fixed.erase(it);
  Strategy _s1 = s;
  _s1.SetAction(State(n), C);
  AssignActions(_s1, to_be_fixed, ans);
  Strategy _s2 = s;
  _s2.SetAction(State(n), D);
  AssignActions(_s2, to_be_fixed, ans);
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
  {
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

  {
    Strategy s("cddd*c*dd*ddcddc*d*d*dcd*dcddcdd*dddcddd**dd**dd*ccd***cdc*cdcdd"); // 3/4 efficient, 1/4 unjudgeable
    std::vector<Strategy> efficients, unjudgeables;
    CheckTopologicalEfficiency(s, efficients, unjudgeables);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
    }
  }

  {
    Strategy s("ccdd**ddc*ccdccdc*ddddccdc****cd*d**ccdcdccddccd**cddd**d*****cd"); // unjudgeable
    std::vector<Strategy> efficients, unjudgeables;
    CheckTopologicalEfficiency(s, efficients, unjudgeables);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
    }
  }
}

template<class T>
void RecursiveCommas(std::ostream& os, T n)
{
  T rest = n % 1000; //"last 3 digits"
  n /= 1000;         //"begining"

  if (n > 0) {
    RecursiveCommas(os, n); //printing "begining"

    //and last chunk
    os << ',' << std::setfill('0') << std::setw(3) << rest;
  }
  else
    os << rest; //first chunk of the number
}


int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  if( argc != 2 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file>" << endl;
    return 1;
  }

  uint64_t n_efficient = 0;
  uint64_t n_unjudgeable = 0;

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << std::endl;
      std::cerr << "n_efficient/n_unjudgeable : ";
      RecursiveCommas(std::cerr, n_efficient);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_unjudgeable);
      std::cerr << std::endl;
    }
    Strategy _str(s.c_str());

    std::vector<Strategy> efficients, unjudgeables;
    CheckTopologicalEfficiency(_str, efficients, unjudgeables);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
      n_efficient += (1 << (64-s.NumFixed()));
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
      n_unjudgeable += (1 << (64-s.NumFixed()));
    }
  }
  std::cerr << "n_efficient/n_unjudgeable : ";
  RecursiveCommas(std::cerr, n_efficient);
  std::cerr << " / ";
  RecursiveCommas(std::cerr, n_unjudgeable);
  std::cerr << std::endl;
  return 0;
#endif
}

