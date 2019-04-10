#include <string>
#include <array>
#include <sstream>
#include <cstdint>
#include <ostream>
#include "Graph.hpp"
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
  State(char str[6]):
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
  };

  State NextState(Action act_a, Action act_b) const {
    return State(a_2,a_1,act_a,b_2,b_1,act_b);
  };

  int RelativePayoff() const {
    else if( a_1 == C && b_1 == D ) { return -1; }
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
    if( b_2 == D ) { id += 1 << 3; }
    if( b_1 == D ) { id += 1 << 2; }
    return id;
  }
};


class Strategy {
public:
  Strategy( std::array<Action,40> acts ); // construct a strategy from a list of actions
  Strategy( const char acts[40] );
  std::array<Action,40> actions;
  std::array<Action,64> fullActions;

  std::string ToString() const;
  friend std::ostream &operator<<(std::ostream &os, const Strategy &strategy);

  Action ActionAt( State fs ) const { return fullActions[fs.ID()]; }
  Action ActionAt( ShortState s ) const { return actions[s.ID()]; };
  bool IsDefensible1() const; // check a necessary condition for defensibility using graph topology
  bool IsDefensible() const; // check necessary sufficient condition for defensibility
  bool IsDistinguishable() const;
  Graph TransitionGraph() const;
  Graph TransitionGraphWithoutPositiveStates() const;
  std::string ToDot() const; // dump in dot format
private:
  void ConstructFullActions();
  void NextPossibleFullStates( State current, std::vector<State>& next_states) const;
  typedef std::array<std::array<int,64>,64> int_matrix_t;
  void ConstructA1Matrix( int_matrix_t& A1_b, int_matrix_t& A1_c ) const;
  int_matrix_t Transpose(const int_matrix_t &a) const;
  void UpdateAMatrix( int_matrix_t& A, const int_matrix_t& A1 ) const;
  bool HasNegativeDiagonal( const int_matrix_t& A ) const;
};

#endif //STRATEGY_HPP

