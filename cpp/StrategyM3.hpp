//
// Created by Yohsuke Murase on 2017/06/26.
//

#include <string>
#include <array>
#include <sstream>
#include <cstdint>
#include <ostream>
#include "Graph.hpp"
#include "Strategy.hpp"
#include "Action.hpp"

#ifndef STRATEGY_M3_HPP
#define STRATEGY_M3_HPP

// FullState: state in the standard si notation
//   a_3,a_2,a_1, b_3,b_2,b_1, c_3,c_2,c_1
//   ccccccccc, etc...
class FullStateM3 {
public:
  FullStateM3(Action _a_3, Action _a_2, Action _a_1, Action _b_3, Action _b_2, Action _b_1, Action _c_3, Action _c_2, Action _c_1):
      a_3(_a_3), a_2(_a_2), a_1(_a_1), b_3(_b_3), b_2(_b_2), b_1(_b_1), c_3(_c_3), c_2(_c_2), c_1(_c_1) {};
  FullStateM3(size_t id):
      a_3( ((id>>8)&1)?D:C ), a_2( ((id>>7)&1)?D:C ), a_1( ((id>>6)&1)?D:C ),
      b_3( ((id>>5)&1)?D:C ), b_2( ((id>>4)&1)?D:C ), b_1( ((id>>3)&1)?D:C ),
      c_3( ((id>>2)&1)?D:C ), c_2( ((id>>1)&1)?D:C ), c_1( ((id>>0)&1)?D:C ) {};
  const Action a_3, a_2, a_1, b_3, b_2, b_1, c_3, c_2, c_1;

  bool operator==(const FullStateM3 & rhs) const {
    return (
    a_3==rhs.a_3 && a_2==rhs.a_2 && a_1==rhs.a_1 &&
    b_3==rhs.b_3 && b_2==rhs.b_2 && b_1==rhs.b_1 &&
    c_3==rhs.c_3 && c_2==rhs.c_2 && c_1==rhs.c_1
    );
  }
  int NumDiffInT1(const FullStateM3& rhs) const {
    if( a_3 != rhs.a_3 || b_3 != rhs.b_3 || c_3 != rhs.c_3 || a_2 != rhs.a_2 || b_2 != rhs.b_2 || c_2 != rhs.c_2 ) { return -1; }
    int count = 0;
    if( a_1 != rhs.a_1 ) { count += 1; }
    if( b_1 != rhs.b_1 ) { count += 1; }
    if( c_1 != rhs.c_1 ) { count += 1; }
    return count;
  }

  friend std::ostream &operator<<(std::ostream &os, const FullStateM3 &state);

  FullStateM3 NextState(Action act_a, Action act_b, Action act_c) const;

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

  FullStateM3 FromB() const { return FullStateM3(b_3,b_2,b_1, a_3,a_2,a_1, c_3,c_2,c_1); } // full state from B's viewpoint
  FullStateM3 FromC() const { return FullStateM3(c_3,c_2,c_1, a_3,a_2,a_1, b_3,b_2,b_1); } // full state from C's viewpoint

  size_t ID() const {  // ID must be 0~63 integer. AllC: 0, AllD: 63
    size_t id = 0;
    if( a_3 == D ) { id += 1 << 8; }
    if( a_2 == D ) { id += 1 << 7; }
    if( a_1 == D ) { id += 1 << 6; }
    if( b_3 == D ) { id += 1 << 5; }
    if( b_2 == D ) { id += 1 << 4; }
    if( b_1 == D ) { id += 1 << 3; }
    if( c_3 == D ) { id += 1 << 2; }
    if( c_2 == D ) { id += 1 << 1; }
    if( c_1 == D ) { id += 1 << 0; }
    return id;
  }
};


class StrategyM3 {
public:
  StrategyM3( const char acts[512] );
  std::array<Action,512> actions;

  std::string ToString() const;
  friend std::ostream &operator<<(std::ostream &os, const StrategyM3 &strategy);

  Action ActionAt( FullStateM3 s ) const { return actions[s.ID()]; }
  bool IsDefensible() const; // check necessary sufficient condition for defensibility
  bool IsDistinguishable() const;
  Graph TransitionGraph() const;
  std::string ToDot() const; // dump in dot format
  static StrategyM3 AllC(); // return AllC strategy
private:
  std::vector<FullStateM3> NextPossibleFullStates( FullStateM3 current ) const;
  typedef std::array<std::array<int,512>,512> a_matrix_t;
  void ConstructA1Matrix( a_matrix_t& A1_b, a_matrix_t& A1_c ) const;
  a_matrix_t Transpose( const a_matrix_t& a ) const;
  void UpdateAMatrix( a_matrix_t& A, const a_matrix_t& A1_t ) const;
  bool HasNegativeDiagonal( const a_matrix_t& A ) const;
};

#endif //STRATEGY_M3_HPP

