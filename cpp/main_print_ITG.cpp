#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <utility>
#include <deque>
#include <sstream>
#include <map>
#include "mpi.h"
#include "Strategy.hpp"

typedef std::map<std::string,int64_t> histo_t;
void Print(const histo_t& counts, const histo_t& total_counts) {
  for(auto kv: counts) {
    std::cout << kv.first << " " << kv.second << " " << total_counts.at(kv.first) << std::endl;
  }
}

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  if( argc != 3 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <strategy_file> <init state>" << std::endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  std::ifstream fin(argv[1]);
  if( !fin.is_open() ) {
    std::cerr << "[Error] No input file " << argv[1] << std::endl;
    throw "no input file";
  }

  const State init_state(argv[2]);
  histo_t counts;
  histo_t total_counts;

  int64_t n = 0;
  for( std::string s; fin >> s; n++) {
    uint64_t n_passed, n_pending, n_rejected;
    fin >> n_passed >> n_pending >> n_rejected;
    if(n % 1000000 == 0) { std::cerr << n << " at rank " << my_rank << std::endl; }
    if(n % num_procs != my_rank) { continue; }
    if(n_passed == 0) { continue; }
    Strategy str(s.c_str());
    // cout << s << endl;
    std::ostringstream oss;
    std::vector<int> histo;
    int current = init_state.ID();
    while( std::find(histo.begin(), histo.end(), current) == histo.end() ) {
      histo.push_back(current);
      oss << current << ",";
      current = str.NextITGState(State(current));
      if(current < 0) { std::cerr << "U" << std::endl; oss << -1; break; }
    }
    std::string key = oss.str();
    if( counts.find(key) == counts.end() ) { counts[key] = 1; }
    else { counts[key]++; }
    if( total_counts.find(key) == total_counts.end() ) { total_counts[key] = n_passed; }
    else { total_counts[key] += n_passed; }
  }
  std::cerr << "result at rank " << my_rank << std::endl;
  Print(counts, total_counts);

  MPI_Finalize();
  return 0;
}

