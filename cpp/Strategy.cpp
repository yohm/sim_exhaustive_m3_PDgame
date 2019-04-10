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

  const size_t N = n;
  std::vector<int> d(N*N);

  // construct adjacency matrix
  const int INFINITE = 10000;
  for(size_t i=0; i<N; i++) {
    for(size_t j=0; j<N; j++) {
      d[i*N+j] = INFINITE;
    }
  }

  for(size_t i=0; i<64; i++) {
    int I = idx[i];
    if(I<0) continue;
    State si(i);
    std::vector<State> sjs;
    NextPossibleStates(si, sjs);
    for( auto sj: sjs) {
      int J = idx[sj.ID()];
      if(J<0) continue;
      d[I*N+J] = sj.RelativePayoff();
    }
  }

  for(size_t i=0; i<N; i++) {
    if(d[i*N+i] < 0) { return false; }
  }

  for(size_t k=0; k<N; k++) {
    for(size_t i=0; i<N; i++) {
      for(size_t j=0; j<N; j++) {
        d[i*N+j] = std::min(d[i*N+j], d[i*N+k]+d[k*N+j]);
      }
      if(d[i*N+i] < 0) { return false; }
    }
  }
  return true;
}

void Strategy::ConstructAdjacencyMatrix(Strategy::int_matrix_t &a) const {
  const int INFINITE = 10000;
  for( size_t i=0; i<64; i++) {
    State si(i);
    for( size_t j=0; j<64; j++) { // initialize with a large payoff
      a[i][j] = INFINITE;
    }
    std::vector<State> sjs;
    NextPossibleStates(si, sjs);
    for( auto sj: sjs) {
      size_t j = sj.ID();
      a[i][j] = sj.RelativePayoff();
    }
  }
}

