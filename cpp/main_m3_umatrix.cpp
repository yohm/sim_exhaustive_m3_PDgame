//
// Created by Yohsuke Murase on 2017/07/26.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "StrategyM3.hpp"
#include "GameM3.hpp"

int main(int argc, char** argv) {

  if( argc != 3 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <e> <strategy>" << std::endl;
    std::cerr << "    example : " << argv[0] << " 0.01 <512bit_strategy_string>" << std::endl;
    return 1;
  }

  double e = std::atof( argv[1] );

  const StrategyM3 str( argv[2] );
  GameM3 g(str, str, str);
  matrix512_t m;
  g.MakeUMatrix(e, m);
  for( auto row: m ) {
    for( auto x: row) {
      std::cout << x << ' ';
    }
    std::cout << std::endl;
  }

  return 0;
}

