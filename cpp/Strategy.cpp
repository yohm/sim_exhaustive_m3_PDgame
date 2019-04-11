#include <iostream>
#include "Strategy.hpp"

Strategy::Strategy(const std::array<Action,64>& acts): actions(acts) {}

Strategy::Strategy(const char acts[64]) {
  for( size_t i=0; i<64; i++) {
    actions[i] = C2A(acts[i]);
  }
}

void Strategy::NextPossibleStates(State current, std::vector<State> &next_states) const {
  Action act_a = ActionAt(current);
  if(act_a == W) {
    next_states.push_back( current.NextState(C,C) );
    next_states.push_back( current.NextState(C,D) );
    next_states.push_back( current.NextState(D,C) );
    next_states.push_back( current.NextState(D,D) );
  }
  else if(act_a == U) {
    // do not insert the next state
  }
  else {
    next_states.push_back( current.NextState(act_a,C) );
    next_states.push_back( current.NextState(act_a,D) );
  }
}

std::ostream &operator<<(std::ostream &os, const Strategy &strategy) {
  for(size_t i=0; i<64; i++) {
    os << strategy.actions[i] << '|' << State(i) << "  ";
    if(i%8 == 7) { os << std::endl; }
  }
  return os;
}

std::string Strategy::ToString() const {
  char c[65];
  for(size_t i=0; i<64; i++) {
    c[i] = A2C(actions[i]);
  }
  c[64] = '\0';
  return std::string(c);
}

bool Strategy::IsDefensible() const {
  // make adjacency graph
  int idx[64];
  int n = 0;
  for(size_t i=0; i<64; i++) {
    if(actions[i] == U) { idx[i] = -1; }
    else { idx[i] = n; n++; }
  }

  d_matrix_t d;

  // construct adjacency matrix
  const size_t N = 64;
  const int INFINITY = 32; // 32 is large enough since the path length is between -16 to 16.
  for(size_t i=0; i<N; i++) {
    for(size_t j=0; j<N; j++) {
      d[i][j] = INFINITY;
    }
  }

  for(size_t i=0; i<N; i++) {
    State si(i);
    std::vector<State> sjs;
    NextPossibleStates(si, sjs);
    for( auto sj: sjs) {
      size_t j = sj.ID();
      d[i][j] = sj.RelativePayoff();
    }
  }

  for(size_t i=0; i<N; i++) {
    if(d[i][i] < 0) { return false; }
  }

  for(size_t k=0; k<N; k++) {
    for(size_t i=0; i<N; i++) {
      for(size_t j=0; j<N; j++) {
        int8_t dikj = d[i][k]+d[k][j];
        d[i][j] = (d[i][j]<dikj) ? d[i][j] : dikj;
      }
      if(d[i][i] < 0) { return false; }
    }
  }
  return true;
}

