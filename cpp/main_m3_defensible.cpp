//
// Created by Yohsuke Murase on 2017/07/05.
//
#include <iostream>
#include <fstream>
#include <cassert>
#include "mpi.h"
#include "StrategyM3.hpp"

int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);

  if( argc != 2 && argc != 3 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <strategy>" << std::endl;
    std::cerr << "     or : " << argv[0] << " <infile_pattern> <outfile_pattern>" << std::endl << std::endl;
    std::cerr << "    example : " << argv[0] << " in_%04d out_%04d" << std::endl;
    return 1;
  }

  int my_rank = 0;
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );

  if( argc == 3 ) {
    std::string in_format = argv[1];
    std::string out_format = argv[2];

    char in_filename[256], out_filename[256];
    sprintf(in_filename, in_format.c_str(), my_rank);
    sprintf(out_filename, out_format.c_str(), my_rank);
    std::cerr << in_filename << " > " << out_filename << " @rank: " << my_rank << std::endl;

    std::ifstream fin(in_filename);
    std::ofstream fout(out_filename);

    std::string line;
    int count = 0, filtered_count = 0;
    while( std::getline(fin,line) ) {
      const StrategyM3 str( line.c_str() );
      if( str.IsDefensible() ) {
        fout << str.ToString() << std::endl;
        filtered_count++;
      }
      count++;
    }
    std::cerr << filtered_count << " / " << count << " @rank "  << my_rank << std::endl;
    fout.close();
  }
  else {
    assert( my_rank == 0 );
    const StrategyM3 str( argv[1] );
    std::cout << "defensible: " << str.IsDefensible() << std::endl;
  }

  MPI_Finalize();

  return 0;
}