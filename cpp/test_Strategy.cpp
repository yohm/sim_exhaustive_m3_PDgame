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
  assert( s1.actions[0] == C );
  assert( s1.actions[7] == D );
  assert( s1.actions[59] == C );
  assert( s1.actions[63] == D );

  std::string bits("ccccddddccccddddccccddddccccddddccccddddccccddddccccddddccccdddd");
  assert( s1.ToString() == bits );
  assert( s1 == Strategy(bits.c_str()) );

  assert( s1.ActionAt(State("cccccc")) == C );
  assert( s1.ActionAt("ddddcc") == D );  // implicit conversion

  Strategy alld("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd");
  assert( alld.IsDefensible() == true );
  Strategy allc("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
  assert( allc.IsDefensible() == false );
  Strategy tft("cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd");
  assert( tft.IsDefensible() == true );
  Strategy wsls("cdcdcdcddcdcdcdccdcdcdcddcdcdcdccdcdcdcddcdcdcdccdcdcdcddcdcdcdc");
  assert( wsls.IsDefensible() == false );
  Strategy tf2t("dddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddcdddc");
  assert( tf2t.IsDefensible() == false );
}

void test_MetaStrategy() {
  Strategy s1("_______d_______c_______d_______d_______c_______d_______c_______d");
  assert( s1.IsDefensible() == true );
  Strategy s2("_______*_______c_______d_______d_______c_______d_______c_______d");
  assert( s2.IsDefensible() == false );

  s1.SetAction("ccdccc", C);
  s1.SetAction("dddccc", W);
  assert( s1.ToString() == "_______dc______c_______d_______d_______c_______d_______c*______d" );
  assert( s1.IsDefensible() );
}

int main() {
  std::cout << "Testing Strategy class" << std::endl;

  test_State();
  test_Strategy();
  test_MetaStrategy();
  return 0;
}

