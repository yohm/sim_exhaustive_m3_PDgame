#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
  if( argc != 2 ) {
    std::cerr << "[Error] invalid number of arguments." << std::endl;
    std::cerr << "  usage: ./find_matching.out <in_strategies>" << std::endl;
    throw std::runtime_error("invalid number of arguments");
  }

  std::ifstream fin(argv[1]);
  if( !fin.is_open() ) {
    std::cerr << "[Error] No input file " << argv[1] << std::endl;
    throw std::runtime_error("no input file");
  }
  std::string s;
  uint64_t sum = 0;
  while( fin >> s ) {
    uint64_t n = 0;
    for(char c : s) {
      if(c == '*' || c == '_') n++;
    }
    sum += (1ULL << n);
  }
  std::cout << sum << std::endl;

  return 0;
}
