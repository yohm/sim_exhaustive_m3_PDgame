#include <iostream>
#include "DirectedGraph.hpp"

void PrintComponents(const components_t& components) {
  for(auto comp: components) {
    for( auto n: comp) {
      std::cout << n << ' ';
    }
    std::cout << "\n";
  }
}

void test_g1() {
  DirectedGraph g1(5);
  g1.AddLink(1, 0);
  g1.AddLink(0, 2);
  g1.AddLink(2, 1);
  g1.AddLink(0, 3);
  g1.AddLink(3, 4);
  components_t components;
  g1.SCCs(components);
  std::cout << "g1:" << std::endl;
  PrintComponents(components);
}

void test_g2() {
  DirectedGraph g(4);
  g.AddLink(0, 1);
  g.AddLink(1, 2);
  g.AddLink(2, 3);
  components_t components;
  g.SCCs(components);
  std::cout << "g2:" << std::endl;
  PrintComponents(components);
}

void test_g3() {
  DirectedGraph g(7);
  g.AddLink(0, 1);
  g.AddLink(1, 2);
  g.AddLink(2, 0);
  g.AddLink(1, 3);
  g.AddLink(1, 4);
  g.AddLink(1, 6);
  g.AddLink(3, 5);
  g.AddLink(4, 5);
  components_t components;
  g.SCCs(components);
  std::cout << "g3:" << std::endl;
  PrintComponents(components);
}

void test_g4() {
  DirectedGraph g(11);
  g.AddLink(0,1);g.AddLink(0,3);
  g.AddLink(1,2);g.AddLink(1,4);
  g.AddLink(2,0);g.AddLink(2,6);
  g.AddLink(3,2);
  g.AddLink(4,5);g.AddLink(4,6);
  g.AddLink(5,6);g.AddLink(5,7);g.AddLink(5,8);g.AddLink(5,9);
  g.AddLink(6,4);
  g.AddLink(7,9);
  g.AddLink(8,9);
  g.AddLink(9,8);
  components_t components;
  g.SCCs(components);
  std::cout << "g4:" << std::endl;
  PrintComponents(components);
}

void test_g5() {
  DirectedGraph g(5);
  g.AddLink(0,1);
  g.AddLink(1,2);
  g.AddLink(2,3);
  g.AddLink(2,4);
  g.AddLink(3,0);
  g.AddLink(4,2);
  components_t components;
  g.SCCs(components);
  std::cout << "g5:" << std::endl;
  PrintComponents(components);
}

void test_transitionNodes1() {
  DirectedGraph g1(5);
  g1.AddLink(1, 0);
  g1.AddLink(0, 2);
  g1.AddLink(2, 1);
  g1.AddLink(0, 3);
  g1.AddLink(3, 3);
  g1.AddLink(3, 4);
  std::cout << "transition nodes in g1':" << std::endl;
  for( auto n: g1.TransitionNodes() ) {
    std::cout << n << ' ';
  }
  std::cout << std::endl;
}

int main( int argc, char* argv[]) {
  DirectedGraph g(5);
  g.AddLink(1,0);
  g.AddLink(0,2);
  g.AddLink(2,1);
  g.AddLink(0,3);
  g.AddLink(3,4);
  g.AddLink(4,4);
  std::cout << g;
  components_t components;
  g.SCCs(components);
  for(auto comp: components) {
    for( auto n: comp) {
      std::cout << n << ' ';
    }
    std::cout << "\n";
  }

  test_g1();
  test_g2();
  test_g3();
  test_g4();
  test_g5();
  test_transitionNodes1();
}