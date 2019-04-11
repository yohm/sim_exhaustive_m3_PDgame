#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <deque>
#include "mpi.h"
#include "Strategy.hpp"

using namespace std;

void Explore(const Strategy& s, const State& init, const vector<Action>& b_moves, vector<Strategy>& found) {
  if( b_moves.empty() ) {
    found.push_back(s);
    return;
  }

  Action a_move = s.ActionAt(init);
  Action b_move = b_moves[0];
  vector<Action> r_b_moves(++b_moves.begin(), b_moves.end() );

  if(a_move == C || a_move == D) {
    State ns = init.NextState(a_move, b_move);
    Explore(s, ns, r_b_moves, found);
  }
  else if(a_move == U) {
    for(int i=0; i<2; i++) {
      Strategy _s = s;
      Action _a_move = (i==0) ? C : D;
      bool defensible = _s.SetActionAndRecalcD(init, _a_move);
      if(!defensible) continue;
      State ns = init.NextState(_a_move, b_move);
      Explore(_s, ns, r_b_moves, found);
    }
  }
  else {
    assert(false);  // not implemented yet
  }
}

vector<Strategy> SelectDefensible(const vector<Strategy>& ins, const State& init, const vector<Action>& b_moves) {
  vector<Strategy> found;
  for(auto in: ins) {
    if(in.IsDefensible()) {
      Explore(in, init, b_moves, found);
    }
  }
  return std::move(found);
}

int main(int argc, char** argv) {

  if( argc != 4 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <init_state> <b_moves>" << endl;
    cerr << "   e.g. : " << argv[0] << " strategies.txt dddddd cddd" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  for( string s; fin >> s; ) {
    ins.push_back(Strategy(s.c_str()));
  }

  State ini(argv[2]);
  vector<Action> b_moves;
  for(int i=0; argv[3][i] != '\0'; i++) {
    b_moves.push_back(C2A(argv[3][i]));
  }

  auto found = SelectDefensible(ins, ini, b_moves);
  for(auto s: found) {
    cout << s.ToString() << endl;
  }
  return 0;
}

