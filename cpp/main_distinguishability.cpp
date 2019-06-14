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

DirectedGraph ConstructG0(const Strategy& str1, const Strategy& str2) {
  // [TODO] implement me

}

void Distinguishable_DFS(const Strategy& strategy, DistinguishabilityResult_t& res) {
  // [TODO] implement me
}

DistinguishabilityResult_t CheckDistinguishability(const Strategy& strategy) {
  DistinguishabilityResult_t res;
  std::vector<Strategy> as = FixL0(strategy);
  for(const Strategy& s: as) {
    Distinguishable_DFS(s, res);
  }
  return res;
}

void test_strategy(const std::string& str, int64_t n_passed = -1, int64_t n_rejected = -1) {
  Strategy s(str.c_str()); // is efficient
  DistinguishabilityResult_t res = CheckDistinguishability(s);
  res.PrintStrategies(std::cout);
  assert( s.Size() == res.n_passed + res.n_rejected );
  if(n_passed>=0) { assert( res.n_passed == n_passed ); }
  if(n_rejected>=0) { assert( res.n_rejected == n_rejected); }
}

void test() {
  test_strategy("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd", 32768,0); // is distinguishable?
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
  int _n;
  for( std::string line; fin >> line >> _n; count++) {
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

      for(const std::string s: res.passed) { passed_out << s << std::endl; }
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

