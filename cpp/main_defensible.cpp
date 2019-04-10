#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "mpi.h"
#include "Strategy.hpp"

int main(int argc, char** argv) {

  if( argc != 4 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <strategy_file> <init_state> <b_moves>" << std::endl;
    std::cerr << "   e.g. : " << argv[0] << " strategies.txt dddddd cddd" << std::endl;
    return 1;
  }

  std::ifstream fin(argv[1]);
  std::vector<std::string> ins;
  for( std::string s; fin >> s; ) {
    ins.push_back(s);
  }

  State ini(argv[2]);
  std::vector<Action> b_moves;
  for(int i=0; argv[3][i] != '\0'; i++) {
    b_moves.push_back(C2A(argv[3][i]));
  }

  std::cout << ins.size() << std::endl;
  std::cout << ini << std::endl;
  for(auto a: b_moves) {
    std::cout << a << ' ';
  }

  return 0;
}

