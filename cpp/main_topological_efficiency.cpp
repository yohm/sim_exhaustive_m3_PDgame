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

uint64_t Num(const Strategy& s) {
  return (1ULL << (64 - s.NumFixed()));
}

// Fix action of s at State n, and return the defensibility
bool FixAction(Strategy& s, long n, Action a) {
  if(s.NumU() > 0) {
    return s.SetActionAndRecalcD(State(n), a);
  }
  else {
    s.SetAction(State(n), a);
    return true;
  }
}

void AssignActions(const Strategy& s, std::set<long> to_be_fixed, std::vector<Strategy>& ans, uint64_t& n_indefensible) {
  if( to_be_fixed.empty() ) {
    ans.push_back(s);
    return;
  }

  auto it = to_be_fixed.begin();
  long n = *it;
  to_be_fixed.erase(it);
  for(int i=0; i<2; i++) {
    Strategy _s = s;
    bool d = FixAction(_s, n, (i==0?C:D));
    if(!d) {
      n_indefensible += Num(_s);
      continue;
    }
    AssignActions(_s, to_be_fixed, ans, n_indefensible);
  }
}

std::vector<Strategy> FixL0(const Strategy& str, uint64_t& n_indefensible) {
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

  std::vector<Strategy> ans;
  AssignActions(str, to_be_fixed, ans, n_indefensible);
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

std::vector<Strategy> FixL1States(const Strategy& str, const DirectedGraph& g, const std::vector<long>& l0, uint64_t& n_indefensible) {
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
  std::function<void(const Strategy&,const std::vector<long>&)> dfs = [&ans,&dfs,&n_indefensible](const Strategy& s, const std::vector<long>& a) {
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
        bool d = FixAction(_s, last, (i==0?C:D));
        if(!d) {
          n_indefensible += Num(_s);
          continue;
        }
        const DirectedGraph _g = _s.ITG();
        std::vector<long> _t = TraceITG(_g, last);
        long _d = _t[_t.size()-1];
        if( _d < 0 ) {
          State sa(-_d);
          State sb = sa.SwapAB();
          if( _s.ActionAt(sa) == U || _s.ActionAt(sa) == W ) { _a.push_back(sa.ID()); }
          if( _s.ActionAt(sb) == U || _s.ActionAt(sb) == W ) { _a.push_back(sb.ID()); }
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

std::vector<long> GetD2(const DirectedGraph& g, const std::vector<long>& d0) {
  // std::vector<long> d0 = {63};
  std::vector<long> d1;
  for(long n: d0) {
    assert(n >= 0);
    for(int i=0; i<2; i++) {
      long _ini = (unsigned long)n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      d1.insert( d1.end(), h.begin(), h.end() );
    }
  }
  std::sort( d1.begin(), d1.end() );
  d1.erase( std::unique(d1.begin(), d1.end()), d1.end() );  // == d1.uniq!

  std::vector<long> d2;
  for(long n: d1) {
    if( n < 0 ) { continue; }
    for(int i=0; i<2; i++) {
      long _ini = (unsigned long)n^((i==0)?1UL:8UL);
      std::vector<long> h = TraceITG(g, _ini);
      d2.insert( d2.end(), h.begin(), h.end() );
    }
  }
  std::sort( d2.begin(), d2.end() );
  d2.erase( std::unique(d2.begin(), d2.end()), d2.end() );  // == d2.uniq!

  return std::move(d2);
}

bool C2notD_AND_D2has0(const DirectedGraph& g, const std::vector<long>& d0) {
  std::vector<long> c2 = GetC2(g);
  assert( std::find_if(c2.begin(), c2.end(), [](long x) { return x<0;}) == c2.end() ); // assert no negative element in c2
  if( !HasCommon(d0,c2) ) {
    // since d0 is not included in c2, at least three-bit errors are needed for the transition (0 -> d0).
    std::vector<long> d2 = GetD2(g, d0);
    if( std::find(d2.begin(), d2.end(), 0) != d2.end() ) {
      // since 0 is included in d2, at most two-bit errors are needed for the transition from (d0 -> 0).
      return true;
    }
  }

  return false;
}

std::vector<Strategy> FixC2States(const Strategy& s, const DirectedGraph& g, uint64_t& n_indefensible) {
  std::vector<long> c2 = GetC2(g);

  std::vector<long> unfixed;
  for(long n: c2) {
    if( n < 0 ) {
      State sa(-n);
      State sb = sa.SwapAB();
      if( s.ActionAt(sa) == U || s.ActionAt(sa) == W ) { unfixed.push_back(sa.ID()); }
      if( s.ActionAt(sb) == U || s.ActionAt(sb) == W ) { unfixed.push_back(sb.ID()); }
    }
  }
  std::sort( unfixed.begin(), unfixed.end() );
  unfixed.erase( std::unique(unfixed.begin(), unfixed.end()), unfixed.end() );

  std::vector<Strategy> ans;
  std::function<void(const Strategy&,const std::vector<long>&)> dfs = [&ans,&dfs,&n_indefensible](const Strategy& s, const std::vector<long>& a) {
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
        bool d = FixAction(_s, last, (i==0?C:D));
        if(!d) {
          n_indefensible += Num(_s);
          continue;
        }
        const DirectedGraph _g = _s.ITG();
        std::vector<long> _t = TraceITG(_g, last);
        long _d = _t[_t.size()-1];
        if( _d < 0 ) {
          State sa(-_d);
          State sb = sa.SwapAB();
          if( _s.ActionAt(sa) == U || _s.ActionAt(sa) == W ) { _a.push_back(sa.ID()); }
          if( _s.ActionAt(sb) == U || _s.ActionAt(sb) == W ) { _a.push_back(sb.ID()); }
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

  dfs(s, unfixed);
  return ans;
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

std::string ToC(uint64_t n) {
  std::ostringstream oss;
  RecursiveCommas(oss, n);
  return oss.str();
}

class Filtered {
public:
  std::vector<Strategy> efficient;
  std::vector<Strategy> pending;
  uint64_t n_rejected;
  Filtered() : n_rejected(0) {};
  uint64_t NumEfficient() const {
    uint64_t n = 0;
    for(const Strategy& s: efficient) { n += Num(s); }
    return n;
  }
  uint64_t NumPending() const {
    uint64_t n = 0;
    for(const Strategy& s: pending) { n += Num(s); }
    return n;
  }
  void PrintStrategies(std::ostream &os) const {
    for(const Strategy& s: efficient) {
      os << "E: " << s.ToString() << std::endl;
    }
    for(const Strategy& s: pending) {
      os << "P: " << s.ToString() << std::endl;
    }
    os << "# E/P/R : " << ToC(NumEfficient()) << " / " << ToC(NumPending()) << " / " << ToC(n_rejected) << std::endl;
  }
};

void JudgeEfficiencyDFS(const Strategy& s, Filtered & f) {
  const DirectedGraph g = s.ITG();
  components_t Lc, Ld, Lu;
  GetLcLdLu(g, Lc, Ld, Lu);

  // All L belongs to Lc
  if( Ld.empty() && Lu.empty() ) {
    f.efficient.push_back(s);
    return;
  }

  // Judge inefficiency
  if( !Ld.empty() ) {
    std::vector<long> c2 = GetC2(g);
    auto found = std::find_if(Ld.begin(), Ld.end(), [&c2](const std::vector<long>& ld) { return HasCommon(ld, c2); } );
    if( found != Ld.end() ) {
      f.n_rejected += Num(s);
      return;
    }
    else {
      DP("Cannot judge by LD");
    }
  }

  if( Lu.empty() ) {
    // Since it is undecidable, FixC2 to narrow down the strategy
    std::vector<Strategy> v_s2 = FixC2States(s, g, f.n_rejected);
    DP("Fixed C2 nodes");
    for(const Strategy& s2: v_s2) {
      const DirectedGraph g2 = s2.ITG();
      std::vector<long> c2 = GetC2( g2 );
      auto found = std::find_if(Ld.begin(), Ld.end(), [&c2](const std::vector<long>& ld) { return HasCommon(ld, c2); } );
      if( found != Ld.end() ) {
        f.n_rejected += Num(s2);
      }
      else if( Ld.size() == 1 && C2notD_AND_D2has0(g2, Ld[0]) ) {
        DP("Judge by C3 and D2");
        f.efficient.push_back(s2);
      }
      else {
        f.pending.push_back(s2);
      }
    }
  }
  else {
    std::vector<Strategy> v_s2 = FixL1States(s, g, Lu[0], f.n_rejected);
    for(const Strategy& s2: v_s2) {
      JudgeEfficiencyDFS(s2, f);
    }
  }
}

void CheckTopologicalEfficiency(Strategy& str, Filtered& f) {
  if( !str.IsDefensible() ) {
    f.n_rejected += Num(str);
    return;
  }
  std::vector<Strategy> assigned = FixL0(str, f.n_rejected);

  for(const Strategy& s: assigned) {
    JudgeEfficiencyDFS(s, f);
  }
}

void test() {
  {
    Strategy s("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd"); // is efficient
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }

  {
    Strategy s("cddd*c*dd*ddcddc*d*d*dcd*dcddcdd*dddcddd**dd**dd*ccd***cdc*cdcdd"); // 3/4 efficient, 1/4 unjudgeable
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccdd**ddc*ccdccdc*ddddccdc****cd*d**ccdcdccddccd**cddd**d*****cd"); // unjudgeable
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccdd*dddc*dcddddccc**cccccdddddd*ddcccdd**c**c****cc***cdcdddddd");
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccddddddccdcddddcccc*cccccdddddddddcccdd*ccccd*d*dcc***ddcdddddd");
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccddcccdc_dcdccdc_ddddcccc____cd_d__ccdcdccddccd__cddd________cd");
    Filtered f;
    CheckTopologicalEfficiency(s, f);
    assert( Num(s) == f.n_rejected + f.NumEfficient() + f.NumPending() );
    f.PrintStrategies(std::cout);
  }
}




int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <out file>" << endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


  ifstream fin(argv[1]);

  std::string out_format = argv[2];
  char outfile[256];
  sprintf(outfile, (out_format+".%d").c_str(), my_rank);
  std::ofstream fout(outfile);

  uint64_t n_efficient = 0;
  uint64_t n_unjudgeable = 0;
  uint64_t n_rejected = 0;

  vector<Strategy> ins;
  int count = 0;
  for( string line; fin >> line; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << " @ " << my_rank << std::endl;
      RecursiveCommas(std::cerr, n_efficient);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_unjudgeable);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_rejected);
      std::cerr << std::endl;
    }

    if( count % num_procs == my_rank ) {
    Strategy _str(line.c_str());

    Filtered f;
    CheckTopologicalEfficiency(_str, f);
    fout << line << ' ' << f.NumEfficient() << ' ' << f.NumPending() << ' ' << f.n_rejected << std::endl;
    n_efficient += f.NumEfficient();
    n_unjudgeable += f.NumPending();
    n_rejected += f.n_rejected;
    }
  }

  uint64_t sum_n_efficient = 0;
  uint64_t sum_n_unjudgeable = 0;
  uint64_t sum_n_rejected = 0;
  MPI_Reduce(&n_efficient, &sum_n_efficient, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_unjudgeable, &sum_n_unjudgeable, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected, &sum_n_rejected, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    std::cerr << "n_efficient/n_unjudgeable/n_rejected : ";
    RecursiveCommas(std::cerr, sum_n_efficient);
    std::cerr << " / ";
    RecursiveCommas(std::cerr, sum_n_unjudgeable);
    std::cerr << " / ";
    RecursiveCommas(std::cerr, sum_n_rejected);
    std::cerr << std::endl;
  }

  MPI_Finalize();

  return 0;
#endif
}

