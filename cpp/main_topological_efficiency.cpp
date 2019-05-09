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
#include "TopologicalEfficiency.hpp"
#include "TraceNegativeDefensible.hpp"

using namespace std;

// defensibilityが確定している戦略に対して、グラフのトポロジーから efficiency が確定できるか判定する
// - ITGを構築する
//   - 未確定ビット(*)が存在する場合は、{c,d}両方の場合のリンクを作成する
//   - ここで構築したグラフのcycleは、すべてのビットが確定した後のcycleをかならず含む。
// - ITGからcycleを検出する
//   - ここではSCCを検出し、そのSCCの中でself-loopが存在するかどうかでcycleを検出することを試みる
// - すべてのcycleが確定ビットのみで構成される場合
//   - すべてのcycleに対して「cycleの1-bit neighborのどれかから、State(0)に必ず到達するようなパスが存在する」が成り立つならば、efficiencyが確定する
// - cycleの中に未確定ビットが含まれている場合、それらのビットを確定して、再帰的に判定する。

void test() {
  {
    Strategy s("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd"); // is efficient
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    Strategy s("cddd*c*dd*ddcddc*d*d*dcd*dcddcdd*dddcddd**dd**dd*ccd***cdc*cdcdd"); // 3/4 efficient, 1/4 unjudgeable
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccdd**ddc*ccdccdc*ddddccdc****cd*d**ccdcdccddccd**cddd**d*****cd"); // unjudgeable
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccdd*dddc*dcddddccc**cccccdddddd*ddcccdd**c**c****cc***cdcdddddd");
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccddddddccdcddddcccc*cccccdddddddddcccdd*ccccd*d*dcc***ddcdddddd");
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    Strategy s("ccddcccdc_dcdccdc_ddddcccc____cd_d__ccdcdccddccd__cddd________cd");
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    // judged by two-bit error tolerance of d0 and c0
    Strategy s("ccddcd_dcdccddcdccddddcddccdcdcdcdd_ccc_dccddcddd_cddd_cddcd__cd");
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }

  {
    // pending
    Strategy s("ccddcd_dccccddcdccccddcddccc_dcdddc_ccc_ddcdddcdd_cddddddccdcddd");
    TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
    assert( s.Size() == res.n_rejected + res.NumEfficient() + res.NumPending() );
    res.PrintStrategies(std::cout);
  }
}




int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <out file>" << endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int num_procs = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


  ifstream fin(argv[1]);

  std::string out_format = argv[2];
  char outfile[256];
  sprintf(outfile, (out_format+".%04d").c_str(), my_rank);
  std::ofstream fout(outfile);
  char outfile2[256];
  sprintf(outfile2, (out_format+".pending.%04d").c_str(), my_rank);
  std::ofstream fout2(outfile2);

  uint64_t n_efficient = 0;
  uint64_t n_unjudgeable = 0;
  uint64_t n_rejected = 0;

  vector<Strategy> ins;
  int count = 0;
  for( string line; fin >> line; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << " @ " << my_rank << std::endl;
      RecursiveCommas(std::cerr, n_efficient);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_unjudgeable);
      std::cerr << " / ";
      RecursiveCommas(std::cerr, n_rejected);
      std::cerr << std::endl;
    }

    if( count % num_procs == my_rank ) {
      auto start = std::chrono::system_clock::now();

      Strategy _str(line.c_str());

      TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(_str);

      auto m1 = std::chrono::system_clock::now();
      double e1 = std::chrono::duration_cast<std::chrono::milliseconds>(m1-start).count();
      if(e1 > 3000.0) { std::cerr << "e1 > 3sec : " << line << std::endl; }

      uint64_t n_passed_d = 0;
      uint64_t n_pending_d = 0;
      uint64_t n_rejected_d = res.n_rejected;
      std::vector<Strategy> v_pending;
      { // check defensibility
        for(const Strategy& s: res.efficient) {
          TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s, 64, 64);
          n_passed_d += res_d.NumDefensible();
          n_rejected_d += res_d.n_rejected;
        }

        auto m2 = std::chrono::system_clock::now();
        double e2 = std::chrono::duration_cast<std::chrono::milliseconds>(m2-m1).count();
        if(e2 > 3000.0) { std::cerr << "e2 > 3sec : " << line << std::endl; }

        for(const Strategy& s: res.pending) {
          TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s, 64, 64);
          n_pending_d += res_d.NumDefensible();
          n_rejected_d += res_d.n_rejected;
          v_pending.insert( v_pending.begin(), res_d.passed.begin(), res_d.passed.end() );
        }
        auto m3 = std::chrono::system_clock::now();
        double e3 = std::chrono::duration_cast<std::chrono::milliseconds>(m3-m2).count();
        if(e3 > 3000.0) { std::cerr << "e3 > 3sec : " << line << std::endl; }
      }
      fout << line << ' ' << n_passed_d << ' ' << n_pending_d << ' ' << n_rejected_d << std::endl;
      n_efficient += n_passed_d;
      n_unjudgeable += n_pending_d;
      n_rejected += n_rejected_d;

      for(const Strategy& s: v_pending) {
        fout2 << s.ToString() << std::endl;
      }
    }
  }
  fout.close();
  fout2.close();

  uint64_t sum_n_efficient = 0;
  uint64_t sum_n_unjudgeable = 0;
  uint64_t sum_n_rejected = 0;
  MPI_Reduce(&n_efficient, &sum_n_efficient, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_unjudgeable, &sum_n_unjudgeable, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected, &sum_n_rejected, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    std::cerr << "n_efficient/n_unjudgeable/n_rejected : ";
    RecursiveCommas(std::cerr, sum_n_efficient);
    std::cerr << " / ";
    RecursiveCommas(std::cerr, sum_n_unjudgeable);
    std::cerr << " / ";
    RecursiveCommas(std::cerr, sum_n_rejected);
    std::cerr << std::endl;
  }

  MPI_Finalize();

  return 0;
#endif
}

