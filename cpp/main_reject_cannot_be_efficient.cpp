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
  if( argc != 2 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file>" << endl;
    return 1;
  }

  int n_passed = 0;
  int n_rejected = 0;

  ifstream fin(argv[1]);
  vector<Strategy> ins;
  int count = 0;
  for( string s; fin >> s; count++ ) {
    if(count % 1000 == 0) {
      std::cerr << "step: " << count << std::endl;
      std::cerr << "passed / rejected : " << n_passed << " / " << n_rejected << std::endl;
    }
    Strategy str(s.c_str());
    if(str.ActionAt("cccccc") != C ) { cerr << str << std::endl;}
    assert(str.ActionAt("cccccc") == C);
    if( str.CannotBeEfficient() ) {
      n_rejected++;
    }
    else {
      n_passed++;
      std::cout << s << std::endl;
    }
  }
  std::cerr << "passed / rejected : " << n_passed << " / " << n_rejected << std::endl;
  return 0;
}

