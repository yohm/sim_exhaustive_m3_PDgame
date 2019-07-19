#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
  if( argc != 2 ) {
    std::cerr << "[Error] invalid number of arguments." << std::endl;
    std::cerr << "  usage: ./find_matching.out <in_strategies>" << std::endl;
    throw "invalid number of arguments";
  }

  std::ifstream fin(argv[1]);
  assert( !fin.fail() );
  std::string s;
  uint64_t sum = 0;
  while( fin >> s ) {
    int n = 0;
    for(int i=0; i<s.length(); i++) {
      if(s[i] == '*' || s[i] == '_') n++;
    }
    sum += (1 << n);
  }
  std::cout << sum << std::endl;

  return 0;
}
