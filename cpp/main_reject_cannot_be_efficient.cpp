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

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <outfile>" << endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  ifstream fin(argv[1]);
  std::string out_format = argv[2];
  char outfile[256];
  sprintf(outfile, (out_format+".%d").c_str(), my_rank);
  std::ofstream fout(outfile);

  int n_passed = 0;
  int n_rejected = 0;

  int count = 0;
  for( string s; fin >> s; count++ ) {
    if(count % 1000 == 0) {
      std::cerr << my_rank << " : step : " << count << std::endl;
      std::cerr << my_rank << " : passed / rejected : " << n_passed << " / " << n_rejected << std::endl;
    }
    if( count % num_procs == my_rank ) {
      Strategy str(s.c_str());
      if(str.ActionAt("cccccc") != C ) { cerr << str << std::endl;}
      assert(str.ActionAt("cccccc") == C);
      if( str.CannotBeEfficient() ) {
        n_rejected++;
      }
      else {
        n_passed++;
        fout << s << std::endl;
      }
    }
  }
  std::cerr << my_rank << " : passed / rejected : " << n_passed << " / " << n_rejected << std::endl;

  int sum_n_passed = 0;
  int sum_n_rejected = 0;
  MPI_Reduce(&n_passed, &sum_n_passed, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected, &sum_n_rejected, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  if( my_rank == 0 ) {
    std::cerr << "[sum] : passed / rejected : " << sum_n_passed << " / " << sum_n_rejected << std::endl;
  }

  MPI_Finalize();
  return 0;
}

