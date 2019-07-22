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
#include <chrono>
#include "mpi.h"
#include "Strategy.hpp"
#include "MyLib.hpp"

// judge distinguishability based on transition graph

class DistinguishabilityResult_t {
public:
  uint64_t n_passed;
  uint64_t n_rejected;
  std::vector<std::string> passed;
  DistinguishabilityResult_t() : n_passed(0), n_rejected(0) {};
  void PrintStrategies(std::ostream &os) const {
    for(const std::string& s: passed) {
      os << s << std::endl;
    }
    os << "# Passed/Rejected : " << ToC(n_passed) << " / " << ToC(n_rejected) << std::endl;
  }
};

std::vector<Strategy> FixL0(const Strategy& strategy) {
  std::vector<int> unfixed;
  for(size_t i=0; i<64; i++) {
    if( (i&7UL) == 0 ) {  // '***ccc'
      if( strategy.actions[i] == U || strategy.actions[i] == W ) {
        unfixed.push_back(i);
      }
    }
  }

  std::vector<Strategy> ans;
  std::function<void(const Strategy&,std::vector<int>)> func = [&ans,&func](const Strategy& s, std::vector<int> to_be_fixed) {
    if(to_be_fixed.empty()) {
      ans.push_back(s);
      return;
    }
    int n = to_be_fixed.back();
    to_be_fixed.pop_back();
    for(int i=0; i<2; i++) {
      Strategy _s = s;
      _s.SetAction( State(n), (i==0)?C:D );
      func(_s, to_be_fixed);
    }
  };
  func(strategy, unfixed);
  return std::move(ans);
}

DirectedGraph ConstructG0(const Strategy& str1, const Strategy& str2, bool make_link_uw = false) {
  DirectedGraph g(64);
  for(int i=0; i<64; i++) {
    State sa(i);
    State sb = sa.SwapAB();
    int j = sb.ID();
    std::vector<Action> acts_a, acts_b;
    if( str1.actions[i] == U || str1.actions[i] == W ) {
      if(make_link_uw) { acts_a.push_back(C); acts_a.push_back(D); }
    }
    else { acts_a.push_back(str1.actions[i]); }
    if( str2.actions[j] == U || str2.actions[j] == W ) {
      if(make_link_uw) { acts_b.push_back(C); acts_b.push_back(D); }
    }
    else { acts_b.push_back(str2.actions[j]); }

    for(Action act_a: acts_a) {
      for(Action act_b: acts_b) {
        int n = sa.NextState(act_a, act_b).ID();
        g.AddLink(i, n);
      }
    }
  }
  return std::move(g);
}

void UpdateGn(DirectedGraph& g) {
  components_t sinks = g.SinkSCCs();

  for(comp_t sink: sinks) {
    if( sink.size() == 1 && g.m_links[sink[0]].size() == 0 ) { continue; } // skip unfixed nodes
    for(long from: sink) {
      assert( g.m_links[from].size() > 0 );  // A node sink SCC must be a fixed node.
      for(int i=0; i<2; i++) {
        long to = from ^ ((i==0)?1UL:8UL);
        if( !g.HasLink(from, to) ) { g.AddLink(from, to); }
      }
    }
  }
}

bool HasNodeWithoutOutlink(const DirectedGraph& g, long init) {
  std::vector<long> ans;
  g.BFS(init, [&ans,&g](long n) {
    if( g.m_links[n].size() == 0 ) {
      ans.push_back(n);
    }
  });
  return (ans.size() > 0);
}

std::vector<Strategy> FixStates(const Strategy& strategy, const Strategy& B_strategy, const DirectedGraph& g, long init) {
  std::vector<long> unfixed;
  g.BFS(init, [&unfixed,&g](long n) {
    if( g.m_links[n].size() == 0 ) {
      unfixed.push_back(n);
    }
  });

  std::vector<Strategy> ans;
  if( unfixed.empty() ) {
    ans.push_back(strategy);
  }
  else {
    long n = unfixed[0];
    for(int i=0; i<2; i++) {
      Strategy _s = strategy;
      DirectedGraph _g = g;
      assert( _s.ActionAt(State(n)) == U || _s.ActionAt(State(n)) == W );
      Action  act_a = (i==0)?C:D;
      State current(n);
      _s.SetAction( current, act_a );
      Action act_b = B_strategy.ActionAt( current.SwapAB() );
      State next_state = current.NextState( act_a, act_b );
      _g.AddLink( current.ID(), next_state.ID() );
      std::vector<Strategy> v = FixStates(_s, B_strategy, _g, init);
      ans.insert( ans.end(), v.begin(), v.end() );
    }
  }
  return std::move(ans);
}

void Distinguishable_DFS(const Strategy& strategy, DistinguishabilityResult_t& res) {
  const Strategy AllC("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
  DirectedGraph g0 = ConstructG0(strategy, AllC, false);
  components_t sccs = g0.NonTransitionComponents();
  std::vector<int> scc_nodes, unjudged;
  for(const comp_t& comp: sccs) {
    if(comp.size() == 1 && comp[0] == 0) { scc_nodes.push_back(comp[0]); }
    else {
      scc_nodes.push_back(comp[0]);
      unjudged.push_back(comp[0]);
    }
  }
  std::sort( scc_nodes.begin(), scc_nodes.end() );
  std::sort( unjudged.begin(), unjudged.end() );

  DirectedGraph gn = g0;
  long errors = 0;

  while(true) {
    if( unjudged.empty() ) {  // indistinguishable
      res.n_rejected += strategy.Size();
      return;
    }

    // At this moment, all SCCs in Gn must be fixed
    UpdateGn(gn);
    errors += 1;

    for(int l : unjudged) {
      if( gn.Reachable(0, l) ) {  // distinguishable
        res.n_passed += strategy.Size();
        res.passed.push_back(strategy.ToString());
        return;
      }
    }

    // fix all actions to nodes that are accessible from scc_nodes
    for(int l: scc_nodes) {
      if( HasNodeWithoutOutlink(gn, l) ) {
        std::vector<Strategy> strategies2 = FixStates(strategy, AllC, gn, l);
        for(const Strategy& s: strategies2) {
          Distinguishable_DFS(s, res);
        }
        return;
      }
    }

    // all SCCs reachable from scc_nodes are now fixed
    std::vector<int> to_be_removed;
    for(int l: unjudged) {
      if( gn.Reachable(l,0) ) {
        to_be_removed.push_back(l);
      }
    }
    for(int n: to_be_removed) {
      auto it = std::find(unjudged.begin(), unjudged.end(), n);
      unjudged.erase(it);
    }
  }
}

DistinguishabilityResult_t CheckDistinguishability(const Strategy& strategy) {
  DistinguishabilityResult_t res;
  std::vector<Strategy> as = FixL0(strategy);
  for(const Strategy& s: as) {
    Distinguishable_DFS(s, res);
  }
  return res;
}

void ExpandWildcard(const Strategy& strategy, std::vector<Strategy>& ans) {
  if( strategy.NumFixed() == 64 ) {
    ans.push_back(strategy);
    return;
  }
  for(int n=0; n<64; n++) {
    Action ai = strategy.ActionAt(n);
    if( ai == U || ai == W ) {
      for(int i=0; i<2; i++) {
        Strategy _s = strategy;
        _s.SetAction( State(n), (i==0)?C:D );
        ExpandWildcard(_s, ans);
      }
      return;
    }
  }
}

void test_strategy(const std::string& str, int64_t n_passed = -1, int64_t n_rejected = -1) {
  Strategy s(str.c_str());
  DistinguishabilityResult_t res = CheckDistinguishability(s);
  res.PrintStrategies(std::cout);
  assert( s.Size() == res.n_passed + res.n_rejected );
  if(n_passed>=0) { assert( res.n_passed == n_passed ); }
  if(n_rejected>=0) { assert( res.n_rejected == n_rejected); }

  std::vector<Strategy> expanded;
  ExpandWildcard(s, expanded);
  int n_passed2 = 0, n_rejected2 = 0;
  for(const Strategy& e: expanded) {
    if( e.IsDistinguishable() ) { n_passed2 ++; }
    else { n_rejected2++; }
  }
  assert( n_passed2 == n_passed );
  assert( n_rejected2 == n_rejected );
}

void test() {
  test_strategy("cdcccdddc*cddcdddcdccd*ccddccdcdcdcd*ccdcdddddddddd*ddddccccddcd", 16,0); // 0 moves to 16(cdcccc)<->40(dcdccc) by 1-bit error
  // test_strategy("cddd*c*dd*dddddcddcd*ddd*ddddcdd*d*dcddddcddccdd**dd***cdc*cdcdd", 1<<13, 0);
  test_strategy("ccddcccdc*dcddcdccc*ddcdcccdddcdcdcdccc*ddcd*cddcccddd*cdcdd**cd", 0, 1<<7);  // indistinguishable. 0/->56(dddccc) & 56->0.
  test_strategy("ccddcccdc*dcddcdccc*ddcdcccdddcdcdcdccc*ddcd*cddcccddd*cdddd**cd", 0, 1<<7);  // G1's sccs (node60) is not fixed. For both expanded strategies, indistinguishable as 0/->56(dddccc) & 56->0.
  test_strategy("cddcccddc*ccd*c*ccddcddddccccdcdcdc*cd**dddddcddddcdddddddcccddd", 1<<6, 0); // second order error must be taken into account. 0->>56 & 56/->0.
}

std::vector<std::string> CompressStrategies(const std::vector<std::string>& strategies) {
  std::vector<std::string> v0( strategies.begin(), strategies.end() );

  bool updated = true;
  while(updated) {
    updated = false;
    std::vector<std::string> v1;
    for(size_t i=0; i<v0.size(); i++) {
      if(i == v0.size()-1) { v1.push_back(v0[i]); break; }
      std::string s1 = v0[i];
      std::string s2 = v0[i+1];
      int diff = 0;
      for(size_t j=0; j<s1.size(); j++) { if(s1[j] != s2[j]) diff++; }
      if( diff == 1 ) {
        int idx = 0;
        for(; idx<s1.size(); idx++) { if(s1[idx] != s2[idx]) break; }
        assert( (s1[idx]=='c'&&s2[idx]=='d') || (s1[idx]=='d'&&s2[idx]=='c') );
        s1[idx] = '*';
        v1.push_back(s1);
        i++;  // i+1 is already checked
        updated = true;
      }
      else {
        v1.push_back(s1);
      }
    }

    // copy v1 to v0
    v0.clear();
    v0.resize(v1.size());
    std::copy(v1.begin(), v1.end(), v0.begin());
  }
  return v0;
}


int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 5 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <in_format> <num_in_files> <out_format> <passed_out_format>" << std::endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  const int N_FILES = std::atoi(argv[2]);
  if(N_FILES <= 0) { throw "invalid input"; }
  const int PROCS_PER_FILE = num_procs / N_FILES;
  char infile[256];
  sprintf(infile, argv[1], my_rank / PROCS_PER_FILE);
  std::cerr << "reading " << infile << " @ rank " << my_rank << std::endl;
  std::ifstream fin(infile);
  if( !fin.is_open() ) {
    std::cerr << "[Error] No input file " << infile << std::endl;
    throw "no input file";
  }

  char outfile[256];
  sprintf(outfile, argv[3], my_rank);
  std::ofstream fout(outfile);
  char outfile2[256];
  sprintf(outfile2, argv[4], my_rank);
  std::ofstream passed_out(outfile2);

  uint64_t n_passed_total = 0;
  uint64_t n_rejected_total = 0;

  long count = 0;
  for( std::string line; fin >> line; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << " @ " << my_rank << std::endl;
      std::cerr << ToC(n_passed_total) << ToC(n_rejected_total) << std::endl;
    }


    if( count % PROCS_PER_FILE == my_rank%PROCS_PER_FILE ) {
      std::cerr << "checking: " << line << std::endl;

      Strategy s1(line.c_str());
      
      auto m0 = std::chrono::system_clock::now();
      DistinguishabilityResult_t res = CheckDistinguishability(s1);
      auto m1 = std::chrono::system_clock::now();
      double e1 = std::chrono::duration_cast<std::chrono::milliseconds>(m1-m0).count();
      if(e1 > 3000.0) {
        std::cerr << "e1 > 3sec : " << line << std::endl;
      }
      fout << line << ' ' << res.n_passed << ' ' << res.n_rejected << std::endl;
      n_passed_total += res.n_passed;
      n_rejected_total += res.n_rejected;

      for(const std::string s: CompressStrategies(res.passed)) { passed_out << s << std::endl; }
    }
  }
  fout.close();
  passed_out.close();

  uint64_t n_passed_all_total = 0;
  uint64_t n_rejected_all_total = 0;
  MPI_Reduce(&n_passed_total, &n_passed_all_total, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected_total, &n_rejected_all_total, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    std::cerr << "n_passed_all_total/n_rejected_all_total : ";
    RecursiveCommas(std::cerr, n_passed_all_total);
    std::cerr << " / ";
    RecursiveCommas(std::cerr, n_rejected_all_total);
    std::cerr << std::endl;
  }

  MPI_Finalize();

  return 0;
#endif
}

