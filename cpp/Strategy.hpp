#include <string>
#include <array>
#include <sstream>
#include <cstdint>
#include <ostream>
#include "Graph.hpp"
#include "Action.hpp"

#ifndef STRATEGY_HPP
#define STRATEGY_HPP


// ShortState: in short notation
//   cc11, cd1-1, dd22 etc...
class ShortState {
public:
  ShortState( Action _a_2, Action _a_1, int8_t _bc_2, int8_t _bc_1 ):
      a_2(_a_2), a_1(_a_1), bc_2(_bc_2), bc_1(_bc_1) {};
  const Action a_2, a_1;
  const int8_t bc_2, bc_1;

  bool operator==(const ShortState & rhs) const {
    return (a_2==rhs.a_2 && a_1==rhs.a_1 && bc_2==rhs.bc_2 && bc_1==rhs.bc_1);
  }

  friend std::ostream &operator<<(std::ostream &os, const ShortState &state);

  static const Action A_STATES[4][2];
  static const int8_t BC_STATES[10][2];
  static const ShortState ALL_STATES[40];

  size_t ID() const {
    for( size_t i=0; i < 40; i++) {
      if( ALL_STATES[i] == *this ) { return i; }
    }
    throw "must not happen";
  }
private:
};

// FullState: state in the standard si notation
//   cccccc, cdcdcd, cddccd etc....
class FullState {
public:
  FullState(Action _a_2, Action _a_1, Action _b_2, Action _b_1, Action _c_2, Action _c_1):
      a_2(_a_2), a_1(_a_1), b_2(_b_2), b_1(_b_1), c_2(_c_2), c_1(_c_1) {};
  FullState(size_t id):
      a_2( ((id>>5)&1)?D:C ), a_1( ((id>>4)&1)?D:C ), b_2( ((id>>3)&1)?D:C ),
      b_1( ((id>>2)&1)?D:C ), c_2( ((id>>1)&1)?D:C ), c_1( ((id>>0)&1)?D:C ) {};
  const Action a_2, a_1, b_2, b_1, c_2, c_1;

  bool operator==(const FullState & rhs) const {
    return (a_2==rhs.a_2 && a_1==rhs.a_1 && b_2==rhs.b_2 && b_1==rhs.b_1 && c_2==rhs.c_2 && c_1&&rhs.c_1);
  }
  int NumDiffInT1(const FullState& rhs) const {
    if( a_2 != rhs.a_2 || b_2 != rhs.b_2 || c_2 != rhs.c_2 ) { return -1; }
    int count = 0;
    if( a_1 != rhs.a_1 ) { count += 1; }
    if( b_1 != rhs.b_1 ) { count += 1; }
    if( c_1 != rhs.c_1 ) { count += 1; }
    return count;
  }

  friend std::ostream &operator<<(std::ostream &os, const FullState &state);

  FullState NextState(Action act_a, Action act_b, Action act_c) const;

  int RelativePayoff() const {
    // returns -2,-1,0,1,2 if the state is very risky, risky, neutral, exploitable, very exploitable
    if( a_1 == C ) {
      if( b_1 == C && c_1 == C ) { return 0; }  // CCC
      else if( b_1 == D && c_1 == D ) { return -2; }  // CDD
      else { return -1; }  // CCD or CDC
    }
    else {
      if( b_1 == C && c_1 == C ) { return 2; }  // DCC
      else if( b_1 == D && c_1 == D ) { return 0; }  // DDD
      else { return 1; }  // DCD or DDC
    }
  }

  int RelativePayoffAgainst(bool BorC) const {
    if( BorC ) { // returns relative payoff against B
      if( a_1 == C && b_1 == D ) { return -1; }
      else if( a_1 == D && b_1 == C ) { return 1; }
      else { return 0; }
    }
    else {
      if( a_1 == C && c_1 == D ) { return -1; }
      else if( a_1 == D && c_1 == C ) { return 1; }
      else { return 0; }
    }
  }

  FullState FromB() const { return FullState(b_2, b_1, a_2, a_1, c_2, c_1); } // full state from B's viewpoint
  FullState FromC() const { return FullState(c_2, c_1, a_2, a_1, b_2, b_1); } // full state from C's viewpoint
  ShortState ToShortState() const {
    int8_t bc_2 = 0;
    if( b_2 == D && c_2 == D ) { bc_2 = 2; }
    else if( b_2 == D || c_2 == D ) { bc_2 = 1; }
    else { bc_2 = 0; }

    int8_t bc_1 = 0;
    if( b_1 == D && c_1 == D ) { bc_1 = 2; }
    else if( b_1 == D || c_1 == D ) {
      if( bc_2 == 1 && b_1 == b_2 ) { bc_1 = -1; }
      else { bc_1 = 1; }
    }
    else { bc_1 = 0 ; }
    return ShortState(a_2,a_1,bc_2,bc_1);
  }

  size_t ID() const {  // ID must be 0~63 integer. AllC: 0, AllD: 63
    size_t id = 0;
    if( a_2 == D ) { id += 1 << 5; }
    if( a_1 == D ) { id += 1 << 4; }
    if( b_2 == D ) { id += 1 << 3; }
    if( b_1 == D ) { id += 1 << 2; }
    if( c_2 == D ) { id += 1 << 1; }
    if( c_1 == D ) { id += 1 << 0; }
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

  Action ActionAt( FullState fs ) const { return fullActions[fs.ID()]; }
  Action ActionAt( ShortState s ) const { return actions[s.ID()]; }
  bool IsDefensible1() const; // check a necessary condition for defensibility using graph topology
  bool IsDefensible() const; // check necessary sufficient condition for defensibility
  bool IsDistinguishable() const;
  Graph TransitionGraph() const;
  Graph TransitionGraphWithoutPositiveStates() const;
  std::string ToDot() const; // dump in dot format
private:
  void ConstructFullActions();
  void NextPossibleFullStates( FullState current, std::vector<FullState>& next_states) const;
  typedef std::array<std::array<int,64>,64> int_matrix_t;
  void ConstructA1Matrix( int_matrix_t& A1_b, int_matrix_t& A1_c ) const;
  int_matrix_t Transpose(const int_matrix_t &a) const;
  void UpdateAMatrix( int_matrix_t& A, const int_matrix_t& A1 ) const;
  bool HasNegativeDiagonal( const int_matrix_t& A ) const;
};

#endif //STRATEGY_HPP

