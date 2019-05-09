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
#include "TraceNegativeDefensible.hpp"

using namespace std;

void test() {
  // Strategy s1("ccddcccd____ccccdcdddd_d___d___d___d___c_d_c_c_d_______c_______d");
  // Strategy s1("c______d______dd_______d____dddd_______d______dd_______ddddddddd");
  // Strategy s1("c______________________________________________________________d");
  // Strategy s1("cd_____dd__d_ddd__c_d__d_c_cdddd__d_c__d_d_c_ccd_d_c_cdd_dcdcd_d");
  // Strategy s1("cdddcccdc_ccdc_c_c_dc__d_c_____dc_c____c_______d___d___c_______d");
  // Strategy s1("cdddcccdc_ccdcdc_cddcddddc__ddddcdcd_cdcdcdddcdd_cdddd_cdcddddcd");
  // Strategy s1("cd_____dc_cd_ddd_cc_d__d___cdddd__d_c__d_d___c_d______dd_d_d_d_d");
  Strategy s1("cdddcccdc_cd_cdc_c_dcdcd____ddddc_c_c__c__d___dd_______c______dd");
  // Strategy s1("cd**cdcdd*cd**cd****cdcc*c****cdcd**cdcc**cd***c***ccd*d******cd");

  TraceNegativeDefensibleResult_t res = TraceNegativeDefensible(s1, 8, 64);
  for(auto s: res.passed) {
    cout << s.ToString() << endl;
  }
  cout << res.passed.size() << " : " << res.NumDefensible() << " / " << res.NumPassed()-res.NumDefensible() << " / " << res.n_rejected << endl;
}

int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 4 && argc != 5 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <out_file> <max_depth> [target num fixed actions]" << endl;
    return 1;
  }

  int n_target_fixed = 64;
  if( argc == 5 ) {
    n_target_fixed = atoi(argv[4]);
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

  int count = 0;
  uint64_t n_determined = 0, n_pending = 0, n_rejected = 0;
  for( string s; fin >> s; count++ ) {
    if(count % 10000 == 0) {
      std::cerr << my_rank << " : step: " << count << std::endl;
      std::cerr << my_rank << " : determined/pending/rejected : " << n_determined << " / " << n_pending << " / " << n_rejected << std::endl;
    }
    if( count % num_procs == my_rank ) {
      Strategy str(s.c_str());
      if(str.ActionAt("cccccc") == U) { str.SetAction("cccccc", C); }
      assert(str.ActionAt("cccccc") == C);
      TraceNegativeDefensibleResult_t res = TraceNegativeDefensible(str, atoi(argv[3]), n_target_fixed);
      for(const auto& s: res.passed) {
        fout << s.ToString() << endl;
      }

      uint64_t n_d = res.DefensibleStrategies().size();
      n_determined += n_d;
      n_pending += (res.passed.size() - n_d);
      n_rejected += res.n_rejection_events;

    }
  }
  std::cerr << my_rank << " : determined/pending/rejected : " << n_determined << " / " << n_pending << " / " << n_rejected << std::endl;

  int sum_n_determined = 0;
  int sum_n_pending = 0;
  int sum_n_rejected = 0;
  MPI_Reduce(&n_determined, &sum_n_determined, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_pending, &sum_n_pending, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected, &sum_n_rejected, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  if( my_rank == 0 ) {
    std::cerr << "[sum] : determined/pending/rejected : " << sum_n_determined << " / " << sum_n_pending << " / " << sum_n_rejected << std::endl;
  }

  MPI_Finalize();

  return 0;
#endif
}

