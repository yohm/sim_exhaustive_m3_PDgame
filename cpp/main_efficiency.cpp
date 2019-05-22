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
#include "MyLib.hpp"


struct Counts {
  int64_t n_efficient;
  int64_t n_inefficient;
  Counts() {
    n_efficient = 0;
    n_inefficient = 0;
  }
};

void EfficiencyDFS(const Strategy& s, Counts& counter) {
  assert( s.NumU() == 0 );
  if( s.NumFixed() == 64 ) {
#ifdef NDEBUG
    if( s.IsEfficientTopo() ) { counter.n_efficient++;}
#else
    bool t = s.IsEfficientTopo();
    bool l = s.IsEfficient();
    assert( t == l );
    if(t) {
      counter.n_efficient++;
    }
#endif
    else { counter.n_inefficient++; }
    return;
  }
  else {
    int idx = 0;
    for(; idx<64; idx++) {
      if(s.ActionAt(idx) == W || s.ActionAt(idx) == U) { break; }
    }
    for(int j=0; j<2; j++) {
      Strategy _s = s;
      _s.SetAction(idx, ((j==0)?C:D) );
      EfficiencyDFS(_s, counter);
    }
  }
}

Counts CheckEfficiency(const Strategy& str) {
  assert( str.NumU() == 0 );
  Counts counter;
  EfficiencyDFS(str, counter);
  return std::move(counter);
}

void testStrategy(const std::string& str, int64_t E_expected = -1, int64_t R_expected = -1) {
  Strategy s(str.c_str());
  auto counter = CheckEfficiency(s);
  if(E_expected >= 0) {
    assert( counter.n_efficient == E_expected );
  }
  if(R_expected >= 0) {
    assert( counter.n_inefficient == R_expected );
  }
  std::cout << "efficient/inefficient : " << counter.n_efficient << " / " << counter.n_inefficient << std::endl;
}

void test() {
  testStrategy("cdddcccdcdcdccdccccdddddcccdccddccddcd*cdccddcddddcd***c****ccdd", 0, 256);  // all inefficient
  testStrategy("ccddcccdccccddcdcdccddccdcddcccd*d*dccddddcdcc**c*ccddcc*c**cccd", 0, 256);  // all inefficient
  // testStrategy("ccdd*c*dc*ccddcdcd*cddccdcdd**cd*d*dccddddcd******ccdd******cccd");  // partly efficient
  // testStrategy("cd*dcd*dd*dd**dcddcd*ddd*ddddcdd*d*dcdcddddcccdd**dd***ccc*cdcdd");
  testStrategy("cdddddddddddccdcddcdddcddddddcddddddcdddcddd**cd**cd***cdc*cdcdd", 256, 0); // all efficient
  testStrategy("cdcdddcdddddccdcddcdddcdcddddcdddcdccdddcddd**cd**cd***cdc*cdcdd", 256, 0); // all efficient
  testStrategy("ccdd*cddc*ccdccdc*ddddccdcc**ccd*dc*ccdcdccddccdcccddd*cdccccccd", 0, 256); // all inefficient
}


int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else
  MPI_Init(&argc, &argv);

  if( argc != 4 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <in_format> <num_files> <out_format>" << std::endl;
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

  char outfile[256];
  sprintf(outfile, argv[3], my_rank);
  std::ofstream fout(outfile);

  uint64_t n_efficient = 0;
  uint64_t n_inefficient = 0;

  int count = 0;
  for( std::string line; fin >> line; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << " @ " << my_rank << " passed/rejected : " << ToC(n_efficient) << " / " << ToC(n_inefficient) << std::endl;
    }

    if( count % PROCS_PER_FILE == my_rank%PROCS_PER_FILE ) {
      std::cerr << "checking: " << line << std::endl;
      auto start = std::chrono::system_clock::now();

      Strategy _str(line.c_str());

      Counts res = CheckEfficiency(_str);

      auto m1 = std::chrono::system_clock::now();
      double e1 = std::chrono::duration_cast<std::chrono::milliseconds>(m1-start).count();
      if(e1 > 3000.0) { std::cerr << "e1 > 3sec : " << line << std::endl; }

      fout << line << ' ' << res.n_efficient << ' ' << res.n_inefficient << std::endl;
      n_efficient += res.n_efficient;
      n_inefficient += res.n_inefficient;
    }
  }
  fout.close();

  uint64_t sum_n_efficient = 0;
  uint64_t sum_n_inefficient = 0;
  MPI_Reduce(&n_efficient, &sum_n_efficient, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_inefficient, &sum_n_inefficient, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    std::cerr << "n_efficient/n_inefficient : " << ToC(sum_n_efficient) << " / " << ToC(sum_n_inefficient) << std::endl;
  }

  MPI_Finalize();

  return 0;
#endif
}

