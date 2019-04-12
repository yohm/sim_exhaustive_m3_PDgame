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

void Explore(const Strategy& s, const State& a_state, const std::set<uint64_t>& histo, int depth, vector<Strategy>& found) {
  if(depth == 0) {
    DP("reached maximum depth");
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
      DP("not defensible: "+std::to_string(depth));
      if(!defensible) continue;
    }
    if(_s.ActionAt(b_state)==U) {
      bool defensible = _s.SetActionAndRecalcD(b_state, ab.second);
      DP("not defensible: "+std::to_string(depth));
      if(!defensible) continue;
    }
    State ns = a_state.NextState(ab.first, ab.second);
    if( ns.ID() == 0 ) { // recovered cooperation
      DP("recovered cooperation: "+std::to_string(depth));
      found.push_back(_s);
      continue;
    }
    else if( histo.find(ns.ID()) != histo.end() || histo.find(ns.SwapAB().ID()) != histo.end() )
    { // loop is detected before recovering full-cooperation
      DP("loop is detected: "+std::to_string(depth));
      continue;
    }
    std::set<uint64_t > n_histo = histo;
    n_histo.insert(ns.ID());
    n_histo.insert(ns.SwapAB().ID());
    Explore(_s, ns, n_histo, depth-1, found);
  }
}

vector<Strategy> SelectEfficientDefensible(Strategy str, int max_depth) {
  const State init("cccccd");
  std::set<uint64_t> histo;
  histo.insert(init.ID());
  histo.insert(init.SwapAB().ID());
  vector<Strategy> found;
  if(str.IsDefensible()) {  // must be called since d_matrix must be prepared
    Explore(str, init, histo, max_depth, found);
  }
  return std::move(found);
}

void test() {
  Strategy s1("ccddcccd____ccccdcdddd_d___d___d___d___c_d_c_c_d_______c_______d");
  // Strategy s1("c______d______dd_______d____dddd_______d______dd_______ddddddddd");
  // Strategy s1("c______________________________________________________________d");
  auto found = SelectEfficientDefensible(s1, 8);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  cout << found.size() << endl;
}

int main(int argc, char** argv) {
  // test();

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <max_depth>" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; ) {
    if(count % 1000 == 0) { std::cerr << "step: " << count << std::endl; }
    Strategy str(s.c_str());
    if(str.ActionAt("cccccc") == U) { str.SetAction("cccccc", C); }
    assert(str.ActionAt("cccccc") == C);
    auto found = SelectEfficientDefensible(str, atoi(argv[2]));
    for(auto s: found) {
      cout << s.ToString() << endl;
    }
    count++;
  }
  return 0;
}

