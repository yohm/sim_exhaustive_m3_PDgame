#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
  if( argc != 3 ) {
    std::cerr << "[Error] invalid number of arguments." << std::endl;
    std::cerr << "  usage: ./find_matching.out <in_strategies> <candidate strategies>" << std::endl;
    throw "invalid number of arguments";
  }

  const int N = 64;

  std::ifstream fin(argv[2]);
  assert( !fin.fail() );
  std::vector<std::string> candidates;
  while( !fin.eof() ) {
    std::string s;
    fin >> s;
    assert(s.length() == N);
    for(int i=0; i<N; i++) { if( s[i] == '_' ) { s[i] = '*'; } }
    candidates.push_back(s);
  }

  std::ifstream fin2(argv[1]);
  assert( !fin2.fail() );
  while( !fin2.eof() ) {
    std::string s;
    fin2 >> s;
    for(int i=0; i<N; i++) { if( s[i] == '_' ) { s[i] = '*'; } }

    std::string found = "NOT_FOUND";
    for(const std::string& cand: candidates) {
      bool matched = true;
      for(int i=0; i<N; i++) {
        if( cand[i] != '*' && s[i] != '*' && cand[i] != s[i] ) { matched = false; break; }
      }
      if( matched ) {
        found = cand;
        break;
      }
    }

    std::cout << s << ' ' << found << std::endl;
    if( found == "NOT_FOUND" ) { throw "matching strategy is not found"; }
  }

  return 0;
}