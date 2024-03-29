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

uint64_t n_recovered = 0;
uint64_t n_pending = 0;
uint64_t n_rejected_by_loop = 0;
uint64_t n_indefensible = 0;
uint64_t n_recovered_event = 0;
uint64_t n_pending_event = 0;
uint64_t n_rejected_by_loop_event = 0;
uint64_t n_indefensible_event = 0;

void Explore(const Strategy& s, const State& a_state, const std::set<uint64_t>& histo, const int depth, vector<Strategy>& found) {
  if(depth == 0) {
    DP("reached maximum depth");
    n_pending += s.Size();
    n_pending_event++;
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
        n_indefensible += _s.Size();
        n_indefensible_event++;
        continue;
      }

    }
    if(_s.ActionAt(b_state)==U) {
      bool defensible = _s.SetActionAndRecalcD(b_state, ab.second);
      if(!defensible) {
        DP("not defensible: "+std::to_string(depth));
        n_indefensible += _s.Size();
        n_indefensible_event++;
        continue;
      }
    }
    State ns = a_state.NextState(ab.first, ab.second);
    if( ns.ID() == 0 ) { // recovered cooperation
      DP("recovered cooperation: "+std::to_string(depth));
      n_recovered += _s.Size();
      n_recovered_event++;
      found.push_back(_s);
      continue;
    }
    else if( histo.find(ns.ID()) != histo.end() || histo.find(ns.SwapAB().ID()) != histo.end() )
    { // loop is detected before recovering full-cooperation
      DP("loop is detected: "+std::to_string(depth));
      n_rejected_by_loop += _s.Size();
      n_rejected_by_loop_event++;
      continue;
    }
    std::set<uint64_t > n_histo = histo;
    n_histo.insert(ns.ID());
    Explore(_s, ns, n_histo, depth-1, found);
  }
}

vector<Strategy> SelectEfficientDefensible(Strategy str, const int max_depth) {
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

Strategy ReplaceWwithU(const Strategy& s) {
  Strategy _s = s;
  for(int i=0; i<64; i++) { if( _s.actions[i] == W ) { _s.actions[i] = U; } }
  return _s;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 4 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <out_file> <max_depth>" << endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  ifstream fin(argv[1]);
  if( !fin.is_open() ) {
    std::cerr << "[Error] No input file : " << argv[1] << std::endl;
    throw std::runtime_error("no input file");
  }

  std::string out_format = argv[2];
  char outfile[256];
  sprintf(outfile, (out_format+".%02d").c_str(), my_rank);
  std::ofstream fout(outfile);

  const int max_depth = strtol(argv[3], nullptr,0);

  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; count++ ) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << std::endl;
      std::cerr << "recovered/pending/indefensible/rejected :" << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;
    }
    if( count % num_procs == my_rank ) {
      Strategy _str(s.c_str());
      Strategy str = ReplaceWwithU(_str);

      assert(str.ActionAt("cccccc") == C);
      auto found = SelectEfficientDefensible(str, max_depth);
      for(const auto& stra: found) {
        fout << stra.ToString() << "\n";
      }
      fout.flush();
    }
  }
  std::cerr << my_rank << " : recovered/pending/indefensible/rejected : " << n_recovered << " / " << n_pending << " / " << n_indefensible << " / " << n_rejected_by_loop << std::endl;

  uint64_t sum_n_recovered = 0;
  uint64_t sum_n_pending = 0;
  uint64_t sum_n_indefensible = 0;
  uint64_t sum_n_rejected = 0;
  MPI_Reduce(&n_recovered, &sum_n_recovered, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_pending, &sum_n_pending, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_indefensible, &sum_n_indefensible, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected_by_loop, &sum_n_rejected, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  if( my_rank == 0 ) {
    std::cerr << "[sum] : recovered/pending/indefensible/rejected : " << sum_n_recovered << " / " << sum_n_pending << " / " << sum_n_indefensible << " / " << sum_n_rejected << std::endl;
  }

  MPI_Finalize();
  return 0;
#endif
}

