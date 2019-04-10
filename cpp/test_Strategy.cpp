#include <iostream>
#include <cassert>
#include "Strategy.hpp"

void test_State() {
  State s("ddcdcd");
  assert(s.a_3 == D);
  assert(s.a_2 == D);
  assert(s.a_1 == C);
  assert(s.b_3 == D);
  assert(s.b_2 == C);
  assert(s.b_1 == D);

  uint64_t id = s.ID();
  assert( id == 53 );

  assert( s == State(id) );

  assert( s.NextState(D,C) == State("dcdcdc") );

  assert( s.RelativePayoff() == -1 );
  assert( State("ddcddc").RelativePayoff() == 0 );
  assert( State("ccdcdc").RelativePayoff() == 1 );

  assert( State("cdcdcd").SwapAB() == State("dcdcdc") );
}

void test_Strategy() {
  const std::array<Action,64> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy s1(acts);

  std::string bits("ccccddddccccddddccccddddccccddddccccddddccccddddccccddddccccdddd");
  assert( s1.ToString() == bits );
  assert( s1 == Strategy(bits.c_str()) );

}

/*
void test_TransitionGraph() {
  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };

  Strategy str(acts);
  Graph g = str.TransitionGraph();
  std::cout << g;

  Graph g2 = str.TransitionGraphWithoutPositiveStates();
  std::cout << g2;

  std::cout << str.ToDot();
}

void test_Defensible() {
  std::cout << "test_Defensible" << std::endl;

  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy str(acts);
  std::cout << str << std::endl;
  std::cout << "  is defensible?" << str.IsDefensible() << std::endl;

  const std::array<Action,40> actsD = {
      C,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D
  };
  Strategy allD(actsD);
  std::cout << allD << std::endl;
  std::cout << "  is defensible?" << allD.IsDefensible() << std::endl;
}


 */

int main() {
  std::cout << "Testing Strategy class" << std::endl;

  test_State();
  test_Strategy();

  /*
  test_TransitionGraph();
  test_Defensible1();
  test_Defensible();

  test_Distinguishable();
   */

  return 0;
}

