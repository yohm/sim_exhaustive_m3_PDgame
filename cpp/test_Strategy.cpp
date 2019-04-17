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

  auto noised = State("ddccdc").NoisedStates();
  assert( noised[0] == State("dddcdc") );
  assert( noised[1] == State("ddccdd") );

  auto prev = State("ddccdc").PossiblePrevStates();
  assert( prev[0] == State("cddccd") );
  assert( prev[1] == State("cdddcd") );
  assert( prev[2] == State("dddccd") );
  assert( prev[3] == State("ddddcd") );
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
  assert( s1.NumFixed() == 64 );

  assert( s1.ActionAt(State("cccccc")) == C );
  assert( s1.ActionAt("ddddcc") == D );  // implicit conversion

  assert( s1.IsDefensible() );
  assert( s1.NegativeDanglingStates().size() == 0 ); // for fixed strategy, there is no dangling states

  {
    Strategy alld("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd");
    assert( alld.IsDefensible() == true );
    assert( alld.IsEfficient() == false );
    auto dests = alld.DestsOfITG();
    for(int i: dests) { assert( i == 63 ); } // all goes to dddddd

    auto stat = alld.StationaryState(0.001);
    for(int i=0; i<63; i++) { assert(stat[i] < 0.01); }
    assert(stat[63] > 0.99);
  }
  {
    Strategy allc("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    assert( allc.IsDefensible() == false );
    assert( allc.IsEfficient() == true );
    auto dests = allc.DestsOfITG();
    for(int i: dests) { assert( i == 0 ); } // all goes to cccccc

    auto stat = allc.StationaryState(0.001);
    for(int i=1; i<64; i++) { assert(stat[i] < 0.01); }
    assert(stat[0] > 0.99);
  }
  {
    Strategy tft("cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd");
    assert( tft.IsDefensible() == true );
    assert( tft.IsEfficient() == false );
    auto dests = tft.DestsOfITG();
    for(int i: dests) { assert( i == 0 || i == 63 || State("cdcdcd").ID() ); } // all goes to either cccccc, dddddd, cdcdcd

    auto stat = tft.StationaryState(0.001);
    assert( abs(stat[0]-0.25) < 0.01 );
    assert( abs(stat[21]-0.25) < 0.01 );
    assert( abs(stat[42]-0.25) < 0.01 );
    assert( abs(stat[63]-0.25) < 0.01 );
  }
  {
    Strategy wsls("cdcdcdcddcdcdcdccdcdcdcddcdcdcdccdcdcdcddcdcdcdccdcdcdcddcdcdcdc");
    assert( wsls.IsDefensible() == false );
    assert( wsls.IsEfficient() == true );
    auto dests = wsls.DestsOfITG();
    for(int i: dests) { assert( i == 0 ); } // all goes to cccccc

    auto stat = wsls.StationaryState(0.001);
    for(int i=1; i<64; i++) { assert(stat[i] < 0.01); }
    assert(stat[0] > 0.99);
  }
  {
    Strategy tf2t("cccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccdcccd"); // tf2t
    assert( tf2t.IsDefensible() == false );
    assert( tf2t.IsEfficient() == true );
    auto dests = tf2t.DestsOfITG();
    for(int i: dests) { assert( i == 0 || i == 63 ); }

    auto stat = tf2t.StationaryState(0.001);
    assert(stat[0] > 0.99);
    for(int i=1; i<64; i++) { assert(stat[i] < 0.01); }
  }
}

void test_MetaStrategy() {
  {
    Strategy s1("_______d_______c_______d_______d_______c_______d_______c_______d");
    assert( s1.IsDefensible() == true );
    assert( s1.NumFixed() == 8 );
    std::vector<State> ds = s1.DanglingStates();
    assert(ds.size() == 7);
    // ds = ["ccdddc", "cdcddc", "dcdddc", "dddddc", "cccddc", "cddddc", "dccddc", "dddddc"]
    //   (<-["cccddd", "ccdddd", "cdcddd", "cddddd", "dccddd", "dcdddd", "ddcddd", "dddddd"])
    //    = ["cccddc", "ccdddc", "cdcddc", "cddddc", "dccddc", "dcdddc", "dddddc"]
    assert(ds[0] == State("cccddc"));
    assert(ds[3] == State("cddddc"));
    assert(ds[6] == State("dddddc"));
    s1.SetAction("ccdccc", C);
    s1.SetAction("dddccc", W);
    assert( s1.ToString() == "_______dc______c_______d_______d_______c_______d_______c*______d" );
    assert( s1.IsDefensible() );
  }

  {
    Strategy s2("_______*_______c_______d_______d_______c_______d_______c_______d");
    assert( s2.IsDefensible() == false );
    auto dests = s2.DestsOfITG();
    for(int i=0; i<63; i++) { assert( dests[i] == -1 ); }  // dests are undetermined
    assert( dests[63] == 63 );  // only dests[63] is determined
  }

  {
    Strategy s3("________________________________________________________________");
    s3.SetAction("ddcddd", D);
    s3.SetAction("dcdddc", D);
    auto ds = s3.DanglingStates();
    // ddcddd - dcdddd
    //        \ dcdddc
    //               \  cdddcc
    //                \ cdddcd
    // dangling states:  dcdddd, cddddc, cddddd
    assert( ds.size() == 3 );
    assert( ds[0] == State("cdddcc"));
    assert( ds[1] == State("cdddcd"));
    assert( ds[2] == State("dcdddd"));

    assert( s3.IsDefensible() );
    auto nds = s3.NegativeDanglingStates();
    assert( nds.size() == 1 );
    assert( nds[0] == State("dcdddd") );
  }
}

void test_EfficiencyDefensible() {
  // Strategy s1("cdddcccdcccdccdccccdddddccccccddccddcdccdccddcddddcdccccccccccdd");
  // Strategy s1("cdddcccdcdcdccdccccdddddccddccddcdddcddcdccddcddddcddddcddddccdd");
  // Strategy s1("cdddcccdc*cdccdccccdddddcc**ccddc*ddcd*cdccddcddddcd***c****ccdd");
  // cdddcccdcdcdccdccccdddddcdcdccddd*ddcd*cdccddcddddcd***ccddddcdd
  // Strategy s1("cdddcccdcdcdccdccccdddddcdcdccdddcddcdccdccddcddddcdcccccddddcdd");
  // Strategy s1("cdddcccdcdcdccdccccdddddcdcdccddddddcddcdccddcddddcddddccddddcdd");
  // cdddcccdcdcdccdcdccdddddcdcdccddd*ddcd*cdccddcddddddc**ccccddcdd
  // Strategy s1("cdddcccdcdcdccdcdccdddddcdcdccdddcddcdccdccddcddddddcccccccddcdd"); // efficient and defensible
  // Strategy s1("cdddcccdcdcdccdcdccdddddcdcdccddddddcddcdccddcddddddcddccccddcdd");
  // Strategy s1("cd*ddd*dcdcd**cd*c*dcd*d**ccdcddddcdcd*d**ddddcd**dd**cd**dcdddd");
  Strategy s1("cdddddddcdcdddcddcddcdddddccdcddddcdcdddddddddcdddddddcddddcdddd");  // efficient and defensible
  // Strategy s1("cdddddddcdcdddcddcddcdddddccdcddddcdcdddddddddcdddddddcddddddddd");
  // Strategy s1("cdcdddcdcdcdcccdcccdcdcdccccdcddddcdcdcdccddddcdccddcccdccdddddd");
  assert( s1.IsEfficient() );
  assert( s1.IsDefensible() );
  // auto stat = s1.StationaryState(0.0001);
  // for(int i=0; i<64; i++) { assert(stat[i] < 0.01); }
}

int main() {
  std::cout << "Testing Strategy class" << std::endl;

  test_State();
  test_Strategy();
  test_MetaStrategy();
  test_EfficiencyDefensible();
  return 0;
}

