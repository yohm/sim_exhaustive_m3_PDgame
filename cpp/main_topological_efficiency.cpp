#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <utility>
#include <deque>
#include <iomanip>
#include <algorithm>
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
    std::sort( neighbors.begin(), neighbors.end() );
    neighbors.erase( std::unique(neighbors.begin(), neighbors.end()), neighbors.end() );
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

// TraceITG until a cycle
// If the path hits an unfixed node, it returns -(unfixed states).
std::vector<long> TraceITG(const DirectedGraph& g, long ini) {
  long current = ini;
  std::vector<long> histo;
  while( std::find(histo.begin(), histo.end(), current) == histo.end() ) {
    histo.push_back(current);
    if( g.m_links[current].size() != 1 ) { // cannot uniquely determine the destination
      histo.push_back(-current);
      break;
    }
    current = g.m_links[current][0];
  }
  return std::move(histo);
}

std::vector<Strategy> FixL1States(const Strategy& str) {
  std::vector<long> unfixed;

  DirectedGraph g = str.ITG();
  components_t comps = g.NonTransitionComponents();
  for(const auto& comp: comps) {
    if( comp.size() == 1 && comp[0] == 0 ) { continue; }  // skip 'cccccc' component
    std::vector<long> neighbors;
    for(unsigned long n: comp) {
      neighbors.push_back(n^1UL);  // 1-bit neighbors
      neighbors.push_back(n^8UL);
    }
    std::sort( neighbors.begin(), neighbors.end() );
    neighbors.erase( std::unique(neighbors.begin(), neighbors.end()), neighbors.end() );

    for(long neigh: neighbors) {
      std::vector<long> _t = TraceITG(g, neigh);
      long d = _t[_t.size()-1];
      if(d < 0) { unfixed.push_back(-d); }
    }
  }
  std::sort( unfixed.begin(), unfixed.end() );
  unfixed.erase( std::unique(unfixed.begin(), unfixed.end()), unfixed.end() );

  std::vector<Strategy> ans;
  std::function<void(const Strategy&,const std::vector<long>&)> dfs = [&ans,&dfs](const Strategy& s, const std::vector<long>& a) {
    if(a.empty()) {
      ans.push_back(s);
      return;
    }
    long last = a[a.size()-1];
    if(s.ActionAt(last) == U || s.ActionAt(last) == W) {
      for(int i=0; i<2; i++) {
        Strategy _s = s;
        std::vector<long> _a = a;
        _a.pop_back();
        _s.SetAction(State(last), (i==0?C:D));
        const DirectedGraph _g = _s.ITG();
        std::vector<long> _t = TraceITG(_g, last);
        long _d = _t[_t.size()-1];
        if( _d < 0 ) { _a.push_back(-_d); }
        dfs(_s, _a);
      }
    }
    else {
      std::vector<long> _a = a;
      _a.pop_back();
      dfs(s, _a);
    }
  };

  dfs(str, unfixed);
  return ans;
}

bool IsSurelyInefficientByC2(const Strategy& str) {
  const DirectedGraph g = str.ITG();
  components_t comps = g.NonTransitionComponents();

  std::vector<long> c0 = {0};
  std::vector<long> c1;
  for(long n: c0) {
    for(int i=0; i<2; i++) {
      long _ini = n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      c1.insert( c1.end(), h.begin(), h.end() );
    }
  }
  std::sort( c1.begin(), c1.end() );
  c1.erase( std::unique(c1.begin(), c1.end()), c1.end() );  // == c1.uniq!

  std::vector<long> c2;
  for(long n: c1) {
    if(n < 0) continue;
    for(int i=0; i<2; i++) {
      long _ini = n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      c2.insert( c2.end(), h.begin(), h.end() );
    }
  }
  std::sort( c2.begin(), c2.end() );
  c2.erase( std::unique(c2.begin(), c2.end()), c2.end() );  // == c2.uniq!

  for(const auto& comp: comps) {
    bool included_in_c2 = false;
    for(long n: comp) {
      if( std::find(c2.begin(),c2.end(), n) != c2.end() ) {
        included_in_c2 = true;
        break;
      }
    }
    if( included_in_c2 ) {
      // trace l1. If none of l1 reaches 0, the strategy is surely inefficient.
      std::vector<long> neighbors;
      for(unsigned long n: comp) {
        neighbors.push_back(n^1UL);  // 1-bit neighbors
        neighbors.push_back(n^8UL);
      }
      std::sort( neighbors.begin(), neighbors.end() );
      neighbors.erase( std::unique(neighbors.begin(), neighbors.end()), neighbors.end() );
      bool return_to_0 = false;
      for(long neigh: neighbors) {
        if( SurelyReach0(g, neigh) ) {
          return_to_0 = true;
          break;
        }
      }
      if( !return_to_0 ) {
        return true;  // surely inefficient
      }
    }
  }
  return false;
}

void CheckTopologicalEfficiency(const Strategy& str, std::vector<Strategy>& efficients, std::vector<Strategy>& unjudgeables) {
  DirectedGraph g = str.ITG();
  components_t comps = g.NonTransitionComponents();

  // fix l0
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
    else if( IsSurelyInefficientByC2(s) ) {
      std::cerr << "rejected by c2" << std::endl;
    }
    else {
      std::vector<Strategy> v_s2 = FixL1States(s);
      for(const Strategy& s2: v_s2) {
        if( IsSurelyEfficient(s2) ) {
          efficients.push_back(s2);
        }
        else {
          if( IsSurelyInefficientByC2(s2) ) {
            std::cerr << "rejected by c2" << std::endl;
            // rejected
          }
          else {
            unjudgeables.push_back(s2);
          }
        }
      }
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

