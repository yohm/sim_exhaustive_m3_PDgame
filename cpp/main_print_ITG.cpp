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
#include "Strategy.hpp"

using namespace std;

#ifdef NDEBUG
#define DP(x) do {} while (0)
#else
#define DP(x) do { std::cerr << x << std::endl; } while (0)
#endif

typedef std::map<std::string,int64_t> histo_t;
void Print(const histo_t& counts, const histo_t& total_counts) {
  for(auto kv: counts) {
    std::cout << kv.first << " : " << kv.second << " : " << total_counts.at(kv.first) << std::endl;
  }
}

int main(int argc, char** argv) {
  if( argc != 3 ) {
    cerr << "Error : invalid argument" << endl;
    cerr << "  Usage : " << argv[0] << " <strategy_file> <init state>" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  histo_t counts;
  histo_t total_counts;
  int n = 0;
  for( std::string s; fin >> s; n++) {
    if(n % 1000000 == 0) { cerr << n << std::endl; }// Print(counts); }
    Strategy str(s.c_str());
    // cout << s << endl;
    std::ostringstream oss;
    State state(argv[2]);
    std::set<int> histo;
    int current = state.ID();
    while( histo.find(current) == histo.end() ) {
      histo.insert(current);
      oss << current << ",";
      // std::cout << State(current) << ":" << current << std::endl;
      current = str.NextITGState(State(current));
      if(current < 0) { std::cout << "U" << endl; oss << -1; break; }
    }
    std::string key = oss.str();
    // if(key == "1,10,20,32,0,") { std::cerr << s << std::endl; }
    if( counts.find(key) == counts.end() ) { counts[key] = 1; }
    else { counts[key]++; }
    int unfixed = 64 - str.NumFixed();
    if( total_counts.find(key) == total_counts.end() ) { total_counts[key] = (1<<unfixed); }
    else { total_counts[key] += (1<<unfixed); }
  }
  std::cerr << "result:" << endl;
  Print(counts, total_counts);

  return 0;
}

