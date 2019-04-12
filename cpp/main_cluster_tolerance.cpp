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

int64_t n_recovered = 0;
int64_t n_pending = 0;

bool CD_clusters_reachable_by_two_bits(const Strategy& s) {
  std::set<uint64_t> c_cluster, d_cluster;
  c_cluster.insert( State("cccccc").ID() );
  uint64_t current_state_id = State("ccdccc").ID();
  while(current_state_id != 0) {
    const State current(current_state_id);
    c_cluster.insert(current.ID());
    c_cluster.insert(current.SwapAB().ID());
    Action a_move = s.ActionAt(current);
    Action b_move = s.ActionAt(current.SwapAB());
    current_state_id = current.NextState(a_move,b_move).ID();
  }

  current_state_id = State("ddcddd").ID();
  while(current_state_id != 63) {
    const State current(current_state_id);
    d_cluster.insert(current.ID());
    d_cluster.insert(current.SwapAB().ID());
    Action a_move = s.ActionAt(current);
    Action b_move = s.ActionAt(current.SwapAB());
    current_state_id = current.NextState(a_move,b_move).ID();
  }

  // c-clusterから1-bitでd-clusterノードに到達できるか調べる
  // d-clusterから1-bitでc-clusterノードに到達できるか調べる
}

void Explore(const Strategy& s, const State& a_state, const std::set<uint64_t>& histo, const int depth, vector<Strategy>& found) {
  if(depth == 0) {
    DP("reached maximum depth");
    n_pending++;
    found.push_back(s);
    return;
  }

  Action a_move = s.ActionAt(a_state);
  const State b_state = a_state.SwapAB();
  Action b_move = s.ActionAt(b_state);

  std::vector<std::pair<Action,Action> > moves;
  if(a_move == U && b_move == U) {
    if(a_state == b_state) {
      moves.emplace_back(C,C);
      moves.emplace_back(D,D);
    }
    else {
      moves.emplace_back(C,C);
      moves.emplace_back(C,D);
      moves.emplace_back(D,C);
      moves.emplace_back(D,D);
    }
  }
  else if(a_move == U) {
    moves.emplace_back(C,b_move);
    moves.emplace_back(D,b_move);
  }
  else if(b_move == U) {
    moves.emplace_back(a_move,C);
    moves.emplace_back(a_move,D);
  }
  else {
    moves.emplace_back(a_move,b_move);
  }

  for(auto ab: moves) {
    Strategy _s = s;
    if(_s.ActionAt(a_state)==U) {
      bool defensible = _s.SetActionAndRecalcD(a_state, ab.first);
      if(!defensible) {
        DP("not defensible: "+std::to_string(depth));
        continue;
      }

    }
    if(_s.ActionAt(b_state)==U) {
      bool defensible = _s.SetActionAndRecalcD(b_state, ab.second);
      if(!defensible) {
        DP("not defensible: "+std::to_string(depth));
        continue;
      }
    }
    State ns = a_state.NextState(ab.first, ab.second);
    if( histo.find(ns.ID()) != histo.end() || histo.find(ns.SwapAB().ID()) != histo.end() )
    { // loop is detected before recovering full-cooperation
      DP("loop is detected: "+std::to_string(depth));
      if(ns.ID() == 63) { // reached 'dddddd', meaning that d-cluster is one-bit tolerant
        DP("d-cluster is one-bit tolerant");
        bool b = CD_clusters_reachable_by_two_bits(_s);
        if(b) {
          DP("c<->d transition occurs with two bit errors");
        }
        else {
          found.push_back(_s);
        }

      }
      else { // didn't reach 'dddddd', meaning that d-cluster is not one-bit tolerant
        DP("d-cluster is not one-bit tolerant")
        found.push_back(_s);
      }
      continue;
    }
    std::set<uint64_t > n_histo = histo;
    n_histo.insert(ns.ID());
    n_histo.insert(ns.SwapAB().ID());
    Explore(_s, ns, n_histo, depth-1, found);
  }
}

vector<Strategy> SelectEfficientDefensible(Strategy str, int max_depth) {
  const State init("dddddc");
  std::set<uint64_t> histo;
  histo.insert(State("dddddd").ID());
  histo.insert(init.ID());
  histo.insert(init.SwapAB().ID());
  vector<Strategy> found;
  if(str.IsDefensible()) {  // must be called since d_matrix must be prepared
    Explore(str, init, histo, max_depth, found);
  }
  return std::move(found);
}

void test() {
  // Strategy s1("ccddcccd____ccccdcdddd_d___d___d___d___c_d_c_c_d_______c_______d");
  // Strategy s1("c______d______dd_______d____dddd_______d______dd_______ddddddddd");
  // Strategy s1("c______________________________________________________________d");
  Strategy s1("cd_____dd__d_ddd__c_d__d_c_cdddd__d_c__d_d_c_ccd_d_c_cdd_dcdcd_d");
  auto found = SelectEfficientDefensible(s1, 1);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  cout << found.size() << endl;
}

int main(int argc, char** argv) {
  // test();
  // return 0;

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
      std::cerr << "recovered/pending :" << n_recovered << " / " << n_pending << std::endl;
    }
    Strategy str(s.c_str());
    if(str.ActionAt("cccccc") == U) { str.SetAction("cccccc", C); }
    assert(str.ActionAt("cccccc") == C);
    auto found = SelectEfficientDefensible(str, atoi(argv[2]));
    for(auto s: found) {
      cout << s.ToString() << endl;
    }
    count++;
  }
  std::cerr << "recovered/pending :" << n_recovered << " / " << n_pending << std::endl;
  return 0;
}

