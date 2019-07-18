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
  uint64_t n_D_E;  // defensible & efficient
  uint64_t n_nD_E; // not defensible & efficient
  uint64_t n_D_nE; // defensible & not efficient
  uint64_t n_nD_nE; // not defensible & not efficient
  Counts() {
    n_D_E = 0;
    n_nD_E = 0;
    n_D_nE = 0;
    n_nD_nE = 0;
  }
};

void CheckDFS(const Strategy& s, Counts& counter) {
  assert( s.NumU() == 0 );
  if( s.NumFixed() == 64 ) {
    bool d = s.IsDefensible2();
    bool e = s.IsEfficientTopo();
#ifndef NDEBUG
    bool l = s.IsEfficient();
    assert(e == l);
#endif
    if( d ) {
      if( e ) { counter.n_D_E++; }
      else { counter.n_D_nE++; }
    }
    else {
      if( e ) { counter.n_nD_E++; }
      else { counter.n_nD_nE++; }
    }
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
      CheckDFS(_s, counter);
    }
  }
}

Counts CheckOneByOne(const Strategy& str) {
  assert( str.NumU() == 0 );
  Counts counter;
  CheckDFS(str, counter);
  return std::move(counter);
}

void testStrategy(const std::string& str, uint64_t D_E = 0, uint64_t D_nE = 0, uint64_t nD_E = 0, uint64_t nD_nE = 0) {
  Strategy s(str.c_str());
  auto c = CheckOneByOne(s);
  if(D_E+D_nE+nD_E+nD_nE >= 0) {
    assert( c.n_D_E == D_E );
    assert( c.n_D_nE == D_nE );
    assert( c.n_nD_E == nD_E );
    assert( c.n_nD_nE == nD_nE );
  }
  std::cout << "D_E / D_nE / nD_E / nD_nE : " << c.n_D_E << " / " << c.n_D_nE << " / " << c.n_nD_E << " / " << c.n_nD_nE << std::endl;
}

void test() {
  testStrategy("cdddcccdcdcdccdccccdddddcccdccddccddcd*cdccddcddddcd***c****ccdd", 0, 256);  // all inefficient
  testStrategy("ccddcccdccccddcdcdccddccdcddcccd*d*dccddddcdcc**c*ccddcc*c**cccd", 0, 256);  // all inefficient
  testStrategy("cdddddddddddccdcddcdddcddddddcddddddcdddcddd**cd**cd***cdc*cdcdd", 256, 0); // all efficient
  testStrategy("cdcdddcdddddccdcddcdddcdcddddcdddcdccdddcddd**cd**cd***cdc*cdcdd", 256, 0); // all efficient
  testStrategy("ccdd*cddc*ccdccdc*ddddccdcc**ccd*dc*ccdcdccddccdcccddd*cdccccccd", 0, 256); // all inefficient
  testStrategy("ccddddddcdccdccdccddddccdccccccddddcccdcdccddccd**cddd*cdc****cd", 0, 128); // inefficient
  testStrategy("ccddcd*dccccddcdccccddcddccc*dcdddc*ccc*ddcdddcdd*cddddddccdcddd", 0, 16, 0, 16); // inefficient
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

      Counts res = CheckOneByOne(_str);

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

