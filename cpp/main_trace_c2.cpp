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
int64_t n_rejected_by_loop = 0;
int64_t n_indefensible = 0;
int64_t n_injudgeable = 0;

void DFS(const Strategy& s, const State& a_state, const std::set<uint64_t>& histo, const int depth, vector<Strategy>& found) {
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
        DP("[NG] not defensible: "+std::to_string(depth));
        n_indefensible++;
        continue;
      }
    }
    if(_s.ActionAt(b_state)==U) {
      bool defensible = _s.SetActionAndRecalcD(b_state, ab.second);
      if(!defensible) {
        DP("[NG] not defensible: "+std::to_string(depth));
        n_indefensible++;
        continue;
      }
    }
    State ns = a_state.NextState(ab.first, ab.second);
    if( ns.ID() == 0 ) { // recovered cooperation
      DP("[OK] recovered cooperation: "+std::to_string(depth));
      n_recovered++;
      found.push_back(_s);
      continue;
    }
    else if( histo.find(ns.ID()) != histo.end() )
    { // loop is detected before recovering full-cooperation
      DP("loop is detected: "+std::to_string(depth));
      if( _s.CannotBeEfficient() ) {
        DP("[NG] cannot be efficient");
        n_rejected_by_loop++;
      }
      else {
        DP("[OK] can be efficient");
        n_pending++;
        found.push_back(_s);
      }
      continue;
    }
    std::set<uint64_t > n_histo = histo;
    n_histo.insert(ns.ID());
    DFS(_s, ns, n_histo, depth-1, found);
  }
}

std::set<int> CollectC1(const Strategy& str) {
  std::set<int> histo;
  const State ini1("cccccd");
  const State ini2("ccdccc");

  histo.insert( 0 );
  histo.insert(ini1.ID());
  int n = str.NextITGState(ini1);
  while( histo.find(n) == histo.end() ) {
    histo.insert(n);
    if( n < 0 ) { break; }
    n = str.NextITGState( State(n) );
  }

  n = str.NextITGState(ini2.ID());
  while( histo.find(n) == histo.end() ) {
    histo.insert(n);
    if( n < 0 ) { break; }
    n = str.NextITGState( State(n) );
  }
  return std::move(histo);

}

vector<Strategy> TraceC2(const Strategy& str, int max_depth) {
  std::set<int> c1 = CollectC1(str);
  if( c1.find(-1) != c1.end() ) { throw "must not contain -1"; }
  std::set<int> c2_start;
  for(int si: c1) {
    for( auto s: State(si).NoisedStates() ) {
      int i = s.ID();
      if( c1.find(i) == c1.end() ) {
        c2_start.insert(i);
      }
    }
  }


  vector<Strategy> in;
  in.push_back(str);
  for(auto init: c2_start) {
    vector<Strategy> out;
    for(auto s: in) {
      if(!s.IsDefensible()) {
        DP("[NG] not defensible");
        n_indefensible++;
        continue;
      }
      std::set<uint64_t> histo;
      histo.insert(init);
      DFS(s, init, histo, max_depth, out);
      std::cerr << "init: " << init << " -> " << out.size() << std::endl;
    }
    out.clear();
    // swap in and out
    // in.clear();
    // in.insert( in.begin(), out.begin(), out.end() );
  }
  return std::move(in);
}

Strategy ReplaceWwithU(const Strategy& s) {
  Strategy _s = s;
  for(int i=0; i<64; i++) { if( _s.actions[i] == W ) { _s.actions[i] = U; } }
  return std::move(_s);
}

void test() {
  // Strategy s1("cd*dcd*dd*cd**cd*d*dcd*c*c****cdcd*d*dcd**cd**dd***ccd********cd");
  Strategy s1("cd*dcd*dd*cd**cd*d*ccd*d*d****cdcd*d*dcd**cd**dd****cd*c***c**cd");
  s1 = ReplaceWwithU(s1);
  auto found = TraceC2(s1, 10);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  std::cerr << "recovered/pending/indefensible/rejected :" << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#endif
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
    auto found = TraceC2(str, atoi(argv[2]));
    for(auto s: found) {
      cout << s.ToString() << endl;
    }
    count++;
  }
  std::cerr << "recovered/pending/indefensible/rejected :" << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;
  return 0;
}

