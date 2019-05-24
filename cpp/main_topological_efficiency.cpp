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
// 方針としてはまずはg0の中のcycleを確定させていく。そのために未確定ビットの両方にリンクをはり、その拡張されたグラフでのSCCを見つける。
// 確定後のITGのcycleは必ずのそのSCCに含まれるので、SCC内のビットを確定していけばcycleは決まる。
// 未確定ビットが多すぎると決めなければならない行動が多くなりすぎて探索空間が爆発するので、その場合はdefensibility checkを先に行って未確定ビットの数をある程度事前に減らしておく。
//
// - ITGを構築する
//   - 未確定ビット(*)が存在する場合は、{c,d}両方の場合のリンクを作成する。ここで構築したITGを未確定ITGと呼ぶことにする。
//   - 一方で全てのアクションが決まった後のITGを確定ITGと呼ぶ。未確定ITG一つに対して確定ITGは2^|*|存在する。
//   - 未確定ITGの中に存在するcycleは、確定ITGのcycleをかならず含む。
// - 未確定ITGからcycle候補となるSCCを検出する
//   - ここではSCCを検出し、その中から「サイズ１かつその中にself-loopが存在しない」ものを除外する。これらのSCCが確定ITGの中でcycleになりうる。
//   - ちなみに確定ITGの中のcycleはかならずsinkSCCになる。
// - 未確定ITGにおけるcycle候補に含まれるノードに未確定ビットが含まれている場合、それらのビットを確定して、再帰的に判定する。
// - すべてのcycle候補が確定ビットのみで構成される場合
//   - この時点で、すべてのcycle候補はcycleになっているはず。
// - g1を構築する
//   - 「State(0)から他のパスには１ビットでいけないが、他のすべてのcycleからはState(0)に１ビットでいける」ならばefficiencyが確定する。 State(0) すべてのcycleに対して「cycleの1-bit neighborのどれかから、State(0)に必ず到達するようなパスが存在する」が成り立つならば、efficiencyが確定する

void test_strategy(const std::string& str, int64_t ED=-1, int64_t E=-1, int64_t P=-1, int64_t R=-1) {
  Strategy s(str.c_str()); // is efficient
  TopologicalEfficiencyResult_t res = CheckTopologicalEfficiency(s);
  res.PrintStrategies(std::cout);
  assert( s.Size() == res.n_efficient_and_defensible + res.n_rejected + res.NumEfficient() + res.NumPending() );
  if(ED>=0) { assert( res.n_efficient_and_defensible == ED ); }
  if( E>=0) { assert( res.NumEfficient() == E); }
  if( P>=0) { assert( res.NumPending() == P); }
  if( R>=0) { assert( res.n_rejected == R); }
}

void test() {
  /*
  test_strategy("cd*d*dddd*dddcdcddcd*cdd*d**dcdd*d*ccddddcddccdd**dd***cdc*cdcdd", 32768,0,0,0); // is efficient
  test_strategy("cddd*c*dd*ddcddc*d*d*dcd*dcddcdd*dddcddd**dd**dd*ccd***cdc*cdcdd", 131072,0,0,0); // all efficient
  test_strategy("ccdd**ddc*ccdccdc*ddddccdc****cd*d**ccdcdccddccd**cddd**d*****cd", 245760,0,0,802816); // partly efficient
  test_strategy("ccddcd_dcdccddcdccddddcddccdcdcdcdd_ccc_dccddcddd_cddd_cddcd__cd", 0,128,0,0);  // efficient
  test_strategy("ccddcd_dccccddcdccccddcddccc_dcdddc_ccc_ddcdddcdd_cddddddccdcddd", 0,0,0,32);  // inefficient
  test_strategy("ccddcdcdd*dcccdddcdd**ddd***cdddccdd***ccc**ccddc**c*****ccdcddd", 131072,0,0,131072);
  test_strategy("cd__cd__c_cc__dc_c__cdddccccddddcdcd______dcdddd__dd__dd__ccccdd", 0,337056,0,3857248);
  test_strategy("cdddcdcdccdddddccc****cd**ccdcdd**dccd********dd**cd**cddc**dcdd", 4194304,0,0,0);
   */
  test_strategy("cddcdcddcccddcdc*ccdddcddcccdccd*dcdcd*dddcdddcd*dcddddddccddddd", 0,0,16,0); // need to consider g2
  test_strategy("cddc*cddc*cddccd*ccddd**cc****dd**c*cddddccddccddddcdd*d**cdcddd");  // g2 must be considered to judge efficiency (many unfixed nodes)
}


int main(int argc, char** argv) {
#ifndef NDEBUG
  test();
  return 0;
#else

  MPI_Init(&argc, &argv);

  if( argc != 5 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <in_format> <num_files> <out_format> <pending_out_format>" << endl;
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
  ifstream fin(infile);
  if( !fin.is_open() ) {
    std::cerr << "[Error] No input file " << infile << std::endl;
    throw "no input file";
  }

  char outfile[256];
  sprintf(outfile, argv[3], my_rank);
  std::ofstream fout(outfile);
  char outfile2[256];
  sprintf(outfile2, argv[4], my_rank);
  std::ofstream fout2(outfile2);

  uint64_t n_efficient_total = 0;
  uint64_t n_unjudgeable_total = 0;
  uint64_t n_rejected_total = 0;

  int count = 0;
  for( string line; fin >> line; count++) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << " @ " << my_rank << std::endl;
      std::cerr << ToC(n_efficient_total) << " / " << ToC(n_unjudgeable_total) << " / " << ToC(n_rejected_total) << std::endl;
    }


    if( count % PROCS_PER_FILE == my_rank%PROCS_PER_FILE ) {
      // std::cerr << "checking: " << line << std::endl;
      uint64_t n_passed = 0;
      uint64_t n_pending = 0;
      uint64_t n_rejected = 0;
      
      Strategy s1(line.c_str());
      
      auto m0 = std::chrono::system_clock::now();
      TopologicalEfficiencyResult_t res1 = CheckTopologicalEfficiency(s1);
      n_passed += res1.n_efficient_and_defensible;
      n_rejected += res1.n_rejected;
      auto m1 = std::chrono::system_clock::now();
      double e1 = std::chrono::duration_cast<std::chrono::milliseconds>(m1-m0).count();
      if(e1 > 3000.0) { std::cerr << "e1 > 3sec : " << line << std::endl; }
      if(e1 > 3000.0) { std::cerr << "  " << res1.efficient.size() << " : " << res1.pending.size() << std::endl; }

      { // check defensibility against efficient strategies
        for(const Strategy& s: res1.efficient) {
          TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s, 64, 64);
          n_passed += res_d.NumDefensible();
          n_rejected += res_d.n_rejected;
        }
        auto m2 = std::chrono::system_clock::now();
        double e2 = std::chrono::duration_cast<std::chrono::milliseconds>(m2-m1).count();
        if(e2 > 3000.0) { std::cerr << "e2 > 3sec : " << line << std::endl; }
      }

      std::vector<std::string> v_pending;
      { // check defensibility against pending strategies
        auto m2 = std::chrono::system_clock::now();
        for(const Strategy& s2: res1.pending) {
          TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s2, 64, 64);
          n_rejected += res_d.n_rejected;
          for(Strategy& s3: res_d.passed) { // we check the efficiency again
            TopologicalEfficiencyResult_t res_e2 = CheckTopologicalEfficiency(s3);
            n_passed += res_e2.n_efficient_and_defensible;
            n_rejected += res_e2.n_rejected;
            for(const Strategy& s4: res_e2.pending) {
              v_pending.push_back(s4.ToString());
              n_pending += s4.Size();
            }
          }
        }
        auto m3 = std::chrono::system_clock::now();
        double e3 = std::chrono::duration_cast<std::chrono::milliseconds>(m3-m2).count();
        if(e3 > 3000.0) { std::cerr << "e3 > 3sec : " << line << std::endl; }
      }
      fout << line << ' ' << n_passed << ' ' << n_pending << ' ' << n_rejected << std::endl;
      n_efficient_total += n_passed;
      n_unjudgeable_total += n_pending;
      n_rejected_total += n_rejected;

      for(const std::string& s: v_pending) {
        fout2 << s << std::endl;
      }
    }
  }
  fout.close();
  fout2.close();

  uint64_t sum_n_efficient = 0;
  uint64_t sum_n_unjudgeable = 0;
  uint64_t sum_n_rejected = 0;
  MPI_Reduce(&n_efficient_total, &sum_n_efficient, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_unjudgeable_total, &sum_n_unjudgeable, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&n_rejected_total, &sum_n_rejected, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if(my_rank == 0) {
    std::cerr << "n_efficient_total/n_unjudgeable_total/n_rejected_total : ";
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

