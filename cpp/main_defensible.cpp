#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "mpi.h"
#include "Strategy.hpp"

int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);

  if( argc != 2 && argc != 3 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <input_file> <output_file>" << std::endl;
    std::cerr << "  or    : " << argv[0] << " <strategy>" << std::endl;
    std::cerr << "  Example : " << argv[0] << " in_%04d.txt out_%04d.txt" << std::endl;
    MPI_Finalize();
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
    std::cerr << "filtering " << in_filename << " > " << out_filename << " @rank: " << my_rank << std::endl;

    std::ifstream fin(in_filename);
    std::ofstream fout(out_filename);

    std::string line;
    int count = 0, filtered_count = 0;
    while( std::getline(fin,line) ) {
      const Strategy str( line.c_str() );
      //std::cout << str.toString() << std::endl;
      //std::cout << str.toFullString() << std::endl;
      if(str.IsDefensible() ) {
        fout << str.ToString() << "\n";
        filtered_count++;
      }
      count++;
#ifndef NDEBUG
      if( count % 1000 == 0 ) { std::cerr << "count: " << count << " @rank: " << my_rank << std::endl; }
#endif
    }
    std::cerr << filtered_count << " strategies are found out of " << count << " at rank " << my_rank << std::endl;
    fout.close();
  }
  else {
    assert( my_rank == 0 );
    const Strategy str( argv[1] );
    if( str.IsDefensible() ) {
      std::cout << "defensible" << std::endl;
    }
    else {
      std::cout << "not defensible" << std::endl;
    }
  }

  MPI_Finalize();

  return 0;
}

