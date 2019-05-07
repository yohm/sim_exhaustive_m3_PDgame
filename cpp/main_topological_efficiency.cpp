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

std::vector<Strategy> FixL0(const Strategy& str) {
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
  std::vector<Strategy> ans;
  AssignActions(str, to_be_fixed, ans);
  return std::move(ans);
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


uint64_t n_rejected = 0;

// Lc : 0, Ld : 1, Lu : 2
int JudgeLType(const DirectedGraph& g, const std::vector<long>& comp) {
  std::vector<long> neighbors;
  for(long n: comp) {
    neighbors.push_back(n^1UL);  // 1-bit neighbors
    neighbors.push_back(n^8UL);
  }
  std::sort( neighbors.begin(), neighbors.end() );
  neighbors.erase( std::unique(neighbors.begin(), neighbors.end()), neighbors.end() );

  bool has_unfixed = false;
  for(long neigh: neighbors) {
    std::vector<long> path = TraceITG(g, neigh);
    long l = path[path.size()-1];
    if( l == 0 ) { return 0; }  // Lc
    else if( l < 0 ) {
      has_unfixed = true;
    }
  }

  return (has_unfixed ? 2 : 1);
}

// get c2.
// node with undetermined actions are included as numbers with negative signs
std::vector<long> GetC2(const DirectedGraph& g) {
  std::vector<long> c0 = {0};
  std::vector<long> c1;
  for(long n: c0) {
    for(int i=0; i<2; i++) {
      long _ini = (unsigned long)n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      assert( h[h.size()-1] == 0 );  // it should return to 0
      c1.insert( c1.end(), h.begin(), h.end() );
    }
  }
  std::sort( c1.begin(), c1.end() );
  c1.erase( std::unique(c1.begin(), c1.end()), c1.end() );  // == c1.uniq!

  std::vector<long> c2;
  for(long n: c1) {
    assert(n >= 0);  // there should be no unfixed bit
    for(int i=0; i<2; i++) {
      long _ini = (unsigned long)n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      c2.insert( c2.end(), h.begin(), h.end() );
    }
  }
  std::sort( c2.begin(), c2.end() );
  c2.erase( std::unique(c2.begin(), c2.end()), c2.end() );  // == c2.uniq!

  return std::move(c2);
}

void GetLcLdLu(const DirectedGraph& g, components_t& Lc, components_t& Ld, components_t& Lu) {
  components_t comps = g.NonTransitionComponents();
  for(const auto& comp: comps) {
    if( comp.size() == 1 && comp[0] == 0 ) { continue; }  // eliminate 'cccccc' component
    int t = JudgeLType(g, comp);
    if( t == 0 ) {
      Lc.push_back(comp);
    }
    else if( t == 1 ) {
      Ld.push_back(comp);
    }
    else if( t == 2 ) {
      Lu.push_back(comp);
    }
    else {
      throw "must not happen";
    }
  }
}

bool HasCommon(const std::vector<long>& s1, std::vector<long>& s2) {
  for(long n: s1) {
    auto found = std::find(s2.begin(), s2.end(), n);
    if( found != s2.end() ) {
      return true;
    }
  }
  return false;
}

std::vector<Strategy> FixL1States(const Strategy& str, const DirectedGraph& g, const std::vector<long>& l0) {
  std::vector<long> l1;
  for(unsigned long n: l0) {
    l1.push_back(n^1UL);  // 1-bit neighbors
    l1.push_back(n^8UL);
  }
  std::sort( l1.begin(), l1.end() );
  l1.erase( std::unique(l1.begin(), l1.end()), l1.end() );

  std::vector<long> unfixed;
  for(long n: l1) {
    std::vector<long> _t = TraceITG(g, n);
    long d = _t[_t.size()-1];
    if(d < 0) {
      State sa(-d);
      State sb = sa.SwapAB();
      if( str.ActionAt(sa) == U || str.ActionAt(sa) == W ) { unfixed.push_back(sa.ID()); }
      if( str.ActionAt(sb) == U || str.ActionAt(sb) == W ) { unfixed.push_back(sb.ID()); }
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
        if( _d < 0 ) {
          State sa(-_d);
          State sb = sa.SwapAB();
          if( s.ActionAt(sa) == U || s.ActionAt(sa) == W ) { _a.push_back(sa.ID()); }
          if( s.ActionAt(sb) == U || s.ActionAt(sb) == W ) { _a.push_back(sb.ID()); }
        }
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


void JudgeEfficiencyDFS(const Strategy& s, std::vector<Strategy>& efficients, std::vector<Strategy>& unjudgeables, std::vector<Strategy>& inefficients) {
  const DirectedGraph g = s.ITG();
  components_t Lc, Ld, Lu;
  GetLcLdLu(g, Lc, Ld, Lu);

  // All L belongs to Lc
  if( Ld.empty() && Lu.empty() ) {
    efficients.push_back(s);
    return;
  }

  // Judge inefficiency
  if( !Ld.empty() ) {
    std::vector<long> c2 = GetC2(g);
    auto found = std::find_if(Ld.begin(), Ld.end(), [&c2](const std::vector<long>& ld) { return HasCommon(ld, c2); } );
    if( found != Ld.end() ) {
      inefficients.push_back(s);
      return;
    }
    else {
      DP("Cannot judge by LD");
    }
  }

  if( Lu.empty() ) {
    // cannot judge efficiency
    unjudgeables.push_back(s);
    return;
  }
  else {
    std::vector<Strategy> v_s2 = FixL1States(s, g, Lu[0]);
    for(const Strategy& s2: v_s2) {
      JudgeEfficiencyDFS(s2, efficients, unjudgeables, inefficients);
    }
  }
}

void CheckTopologicalEfficiency(const Strategy& str, std::vector<Strategy>& efficients, std::vector<Strategy>& unjudgeables, std::vector<Strategy>& inefficients) {
  std::vector<Strategy> assigned = FixL0(str);

  for(const Strategy& s: assigned) {
    JudgeEfficiencyDFS(s, efficients, unjudgeables, inefficients);
  }
}

void test() {
  {
    Strategy s("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd"); // is efficient
    std::vector<Strategy> efficients, unjudgeables, inefficients;
    CheckTopologicalEfficiency(s, efficients, unjudgeables, inefficients);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
    }
    for(auto s: inefficients) {
      cout << "I: " << s.ToString() << endl;
    }
  }

  {
    Strategy s("cddd*c*dd*ddcddc*d*d*dcd*dcddcdd*dddcddd**dd**dd*ccd***cdc*cdcdd"); // 3/4 efficient, 1/4 unjudgeable
    std::vector<Strategy> efficients, unjudgeables, inefficients;
    CheckTopologicalEfficiency(s, efficients, unjudgeables, inefficients);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
    }
    for(auto s: inefficients) {
      cout << "I: " << s.ToString() << endl;
    }
  }

  {
    Strategy s("ccdd**ddc*ccdccdc*ddddccdc****cd*d**ccdcdccddccd**cddd**d*****cd"); // unjudgeable
    std::vector<Strategy> efficients, unjudgeables, inefficients;
    CheckTopologicalEfficiency(s, efficients, unjudgeables, inefficients);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
    }
    for(auto s: inefficients) {
      cout << "I: " << s.ToString() << endl;
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
      std::cerr << "n_efficient/n_unjudgeable/n_rejected : ";
      RecursiveCommas(std::cerr, n_efficient);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_unjudgeable);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_rejected);
      std::cerr << std::endl;
    }
    Strategy _str(s.c_str());

    std::vector<Strategy> efficients, unjudgeables, inefficients;
    CheckTopologicalEfficiency(_str, efficients, unjudgeables, inefficients);
    for(auto s: efficients) {
      cout << "E: " << s.ToString() << endl;
      n_efficient += (1 << (64-s.NumFixed()));
    }
    for(auto s: unjudgeables) {
      cout << "U: " << s.ToString() << endl;
      n_unjudgeable += (1 << (64-s.NumFixed()));
    }
  }
  std::cerr << "n_efficient/n_unjudgeable/n_rejected : ";
  RecursiveCommas(std::cerr, n_efficient);
  std::cerr << " / ";
  RecursiveCommas(std::cerr, n_unjudgeable);
  std::cerr << " / ";
  RecursiveCommas(std::cerr, n_rejected);
  std::cerr << std::endl;
  return 0;
#endif
}

