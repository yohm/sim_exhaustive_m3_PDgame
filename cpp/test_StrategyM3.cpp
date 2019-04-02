//
// Created by Yohsuke Murase on 2017/06/26.
//

#include <iostream>
#include <fstream>
#include <cassert>
#include "StrategyM3.hpp"

void expect(bool b) {
  if(b==false) {
    throw "test failed";
  }
}

void test_StateM3() {

  FullStateM3 s(D,D,C,D,D,D,C,C,C);

  std::cout << "state: " << s;
  std::cout << "  restored_from_id: " << FullStateM3(s.ID() );
  std::cout << std::endl;

  std::cout << "from B: " << s.FromB();
  std::cout << std::endl;
  std::cout << "from C: " << s.FromC();
  std::cout << std::endl;

  expect( s.NumDiffInT1(s) == 0 );

  FullStateM3 s2(D,D,D,D,D,D,C,C,C);
  expect( s.NumDiffInT1(s2) == 1 );

  FullStateM3 s3(D,D,D,D,D,C,C,C,C);
  expect( s.NumDiffInT1(s3) == 2 );

  FullStateM3 s4(D,D,D,D,D,C,C,C,D);
  expect( s.NumDiffInT1(s4) == 3 );

  FullStateM3 s5(D,D,C,D,D,D,C,D,C);
  expect( s.NumDiffInT1(s5) == -1 );
}

void test_StrategyM3() {
  const char* acts =
      "cdcdcdcdddccdddccccdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddccddcdddcdcdddcddcdddcddddddddddcdddcdddddcddd"
      "cddcdddcdddddddddddccdddcdcccddcddcccccdccddccddccddcdddcddcdddcddcdcccdccddccddccccddccddccddcdddddddcddddddd"
      "ddcdcccdccddcdddcddddddcddddddddddddcdcdcdcddddcdddccdcdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddcdddcddddd"
      "cdddcddccddcddddddddddcdddcdddddcdddcddcdddcddddddddddddcdddcddcdddcddcdcccdccddccddccddcdddcddcdddcddcdcccdcc"
      "ddccddccccddccddcdddcdddddddddddddddddddccddccddcdddcddddddddddddddddddd";

  StrategyM3 str(acts);
  std::cout << "strategy:\n" << str << std::endl;
}

void test_TransitionGraph() {

  const char* acts =
      "cdcdcdcdddccdddccccdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddccddcdddcdcdddcddcdddcddddddddddcdddcdddddcddd"
          "cddcdddcdddddddddddccdddcdcccddcddcccccdccddccddccddcdddcddcdddcddcdcccdccddccddccccddccddccddcdddddddcddddddd"
          "ddcdcccdccddcdddcddddddcddddddddddddcdcdcdcddddcdddccdcdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddcdddcddddd"
          "cdddcddccddcddddddddddcdddcdddddcdddcddcdddcddddddddddddcdddcddcdddcddcdcccdccddccddccddcdddcddcdddcddcdcccdcc"
          "ddccddccccddccddcdddcdddddddddddddddddddccddccddcdddcddddddddddddddddddd";

  StrategyM3 str(acts);
  Graph g = str.TransitionGraph();
  expect( g.m_num_nodes == 512 );
  expect( g.m_links.size() == 512 );
  for( const auto& links_i : g.m_links ) {
    expect(links_i.size() == 4);
  }

}

void test_Defensible() {
  std::cerr << "test defensibility" << std::endl;
  const char* acts =
      "cdcdcdcdddccdddccccdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddccddcdddcdcdddcddcdddcddddddddddcdddcdddddcddd"
          "cddcdddcdddddddddddccdddcdcccddcddcccccdccddccddccddcdddcddcdddcddcdcccdccddccddccccddccddccddcdddddddcddddddd"
          "ddcdcccdccddcdddcddddddcddddddddddddcdcdcdcddddcdddccdcdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddcdddcddddd"
          "cdddcddccddcddddddddddcdddcdddddcdddcddcdddcddddddddddddcdddcddcdddcddcdcccdccddccddccddcdddcddcdddcddcdcccdcc"
          "ddccddccccddccddcdddcdddddddddddddddddddccddccddcdddcddddddddddddddddddd";

  StrategyM3 str(acts);
  expect( str.IsDefensible() == true );

  std::cerr << "test defensibility allC" << std::endl;
  const char* actsC =
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
  StrategyM3 allC(actsC);
  expect( allC.IsDefensible() == false );

  std::cerr << "test defensibility allD" << std::endl;
  const char* actsD =
      "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
          "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
          "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
          "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";
  StrategyM3 allD(actsD);
  expect( allD.IsDefensible() == true );
}

void test_AllC() {
  const StrategyM3 allc = StrategyM3::AllC();
  std::string allc_str =
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
  std::cerr << "test AllC" << std::endl;
  expect( allc.IsDefensible() == false );
  expect( allc.ToString() == allc_str );
}

int main() {
  std::cout << "Testing StrategyM3 class" << std::endl;

  test_StateM3();
  test_StrategyM3();

  test_TransitionGraph();
  test_Defensible();
  test_AllC();
  return 0;
}

