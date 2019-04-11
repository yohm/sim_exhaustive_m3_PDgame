#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <cstdint>
#include <ostream>
#include "Action.hpp"

#ifndef STRATEGY_HPP
#define STRATEGY_HPP


class State {
public:
  State(Action _a_3, Action _a_2, Action _a_1, Action _b_3, Action _b_2, Action _b_1):
      a_3(_a_3), a_2(_a_2), a_1(_a_1), b_3(_b_3), b_2(_b_2), b_1(_b_1) {assert(AllCorD());};
  State(uint64_t id):  // upper bit [a_3,a_2,a_1,b_3,b_2,b_1] lower bit
      a_3( ((id>>5)&1)?D:C ), a_2( ((id>>4)&1)?D:C ), a_1( ((id>>3)&1)?D:C ),
      b_3( ((id>>2)&1)?D:C ), b_2( ((id>>1)&1)?D:C ), b_1( ((id>>0)&1)?D:C ) { assert(AllCorD());};
  State(const char str[6]):
      a_3(C2A(str[0])), a_2(C2A(str[1])), a_1(C2A(str[2])),
      b_3(C2A(str[3])), b_2(C2A(str[4])), b_1(C2A(str[5])) {assert(AllCorD());};
  const Action a_3, a_2, a_1, b_3, b_2, b_1;
  bool AllCorD() const {
    return (
        (a_3==D || a_3==C) && (a_2==D || a_2==C) && (a_1==D || a_1==C) &&
        (b_3==D || b_3==C) && (b_2==D || b_2==C) && (b_1==D || b_1==C)
    );
  }

  bool operator==(const State & rhs) const {
    return (a_3==rhs.a_3 && a_2==rhs.a_2 && a_1==rhs.a_1 && b_3==rhs.b_3 && b_2==rhs.b_2 && b_1==rhs.b_1);
  }

  friend std::ostream &operator<<(std::ostream &os, const State &s) {
    os << s.a_3 << s.a_2 << s.a_1 << s.b_3 << s.b_2 << s.b_1;
    return os;
  };

  State NextState(Action act_a, Action act_b) const {
    return State(a_2,a_1,act_a,b_2,b_1,act_b);
  };

  int RelativePayoff() const {
    if( a_1 == C && b_1 == D ) { return -1; }
    else if( a_1 == D && b_1 == C ) { return  1; }
    else if( a_1 == b_1 ) { return 0; }
    else { assert(false); return -10000; }
  }

  State SwapAB() const { return State(b_3, b_2, b_1, a_3, a_2, a_1); } // state from B's viewpoint

  uint64_t ID() const {  // ID must be 0~63 integer. AllC: 0, AllD: 63
    uint64_t id = 0;
    if( a_3 == D ) { id += 1 << 5; }
    if( a_2 == D ) { id += 1 << 4; }
    if( a_1 == D ) { id += 1 << 3; }
    if( b_3 == D ) { id += 1 << 2; }
    if( b_2 == D ) { id += 1 << 1; }
    if( b_1 == D ) { id += 1 << 0; }
    return id;
  }
};


class Strategy {
public:
  Strategy( const std::array<Action,64>& acts ); // construct a strategy from a list of actions
  Strategy( const char acts[64] );
  std::array<Action,64> actions;

  std::string ToString() const;
  friend std::ostream &operator<<(std::ostream &os, const Strategy &strategy);
  bool operator==(const Strategy & rhs) const {
    for(size_t i=0; i<64; i++) { if(actions[i] != rhs.actions[i]) return false; }
    return true;
  }

  Action ActionAt( const State& s ) const { return actions[s.ID()]; }
  void SetAction( const State& s, Action a ) { assert(actions[s.ID()]==U); actions[s.ID()] = a; }
  bool IsDefensible();  // check defensibility. If defensible, m_d is also calculated
  bool SetActionAndRecalcD(const State& s, Action a); // set action[s]=a, and recalculate `m_d`. If not defensible, return false.
private:
  typedef std::array<std::array<int8_t,64>,64> d_matrix_t;
  d_matrix_t m_d;
  bool d_matrix_ready;
  std::vector<State> NextPossibleStates(State current) const;
};

#endif //STRATEGY_HPP

