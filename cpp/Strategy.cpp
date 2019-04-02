//
// Created by Yohsuke Muraes on 2017/05/22.
//

#include <iostream>
#include <set>
#include "Strategy.hpp"


const Action ShortState::A_STATES[4][2] = {
      {C,C},
      {C,D},
      {D,C},
      {D,D}
  };

const int8_t ShortState::BC_STATES[10][2] = {
    {0, 0},
    {0, 1},
    {0, 2},
    {1, 0},
    {1, 1},
    {1,-1},
    {1, 2},
    {2, 0},
    {2, 1},
    {2, 2}
};

const ShortState ShortState::ALL_STATES[40] = {
    ShortState( C, C, 0, 0 ),
    ShortState( C, C, 0, 1 ),
    ShortState( C, C, 0, 2 ),
    ShortState( C, C, 1, 0 ),
    ShortState( C, C, 1, 1 ),
    ShortState( C, C, 1, -1 ),
    ShortState( C, C, 1, 2 ),
    ShortState( C, C, 2, 0 ),
    ShortState( C, C, 2, 1 ),
    ShortState( C, C, 2, 2 ),
    ShortState( C, D, 0, 0 ),
    ShortState( C, D, 0, 1 ),
    ShortState( C, D, 0, 2 ),
    ShortState( C, D, 1, 0 ),
    ShortState( C, D, 1, 1 ),
    ShortState( C, D, 1, -1 ),
    ShortState( C, D, 1, 2 ),
    ShortState( C, D, 2, 0 ),
    ShortState( C, D, 2, 1 ),
    ShortState( C, D, 2, 2 ),
    ShortState( D, C, 0, 0 ),
    ShortState( D, C, 0, 1 ),
    ShortState( D, C, 0, 2 ),
    ShortState( D, C, 1, 0 ),
    ShortState( D, C, 1, 1 ),
    ShortState( D, C, 1, -1 ),
    ShortState( D, C, 1, 2 ),
    ShortState( D, C, 2, 0 ),
    ShortState( D, C, 2, 1 ),
    ShortState( D, C, 2, 2 ),
    ShortState( D, D, 0, 0 ),
    ShortState( D, D, 0, 1 ),
    ShortState( D, D, 0, 2 ),
    ShortState( D, D, 1, 0 ),
    ShortState( D, D, 1, 1 ),
    ShortState( D, D, 1, -1 ),
    ShortState( D, D, 1, 2 ),
    ShortState( D, D, 2, 0 ),
    ShortState( D, D, 2, 1 ),
    ShortState( D, D, 2, 2 )
};

Strategy::Strategy(std::array<Action,40> acts): actions(acts) { ConstructFullActions(); }

Strategy::Strategy(const char *acts) {
  for( size_t i=0; i<40; i++) {
    actions[i] = (acts[i] == 'c' ? C : D);
  }
  ConstructFullActions();
}

void Strategy::ConstructFullActions() {
  for( size_t i=0; i<64; i++) {
    FullState fs(i);
    ShortState ss = fs.ToShortState();
    fullActions[i] = actions[ss.ID()];
  }
}

Graph Strategy::TransitionGraph() const {
  Graph g(64);
  for( size_t i=0; i<64; i++) {
    FullState fs(i);
    std::vector<FullState> next_states;
    NextPossibleFullStates( fs, next_states);
    for( auto next_s: next_states) {
      size_t u = fs.ID();
      size_t v = next_s.ID();
      g.AddLink(u,v);
    }
  }
  return std::move(g);
}

void Strategy::NextPossibleFullStates(FullState current, std::vector<FullState> &next_states) const {
  Action act_a = ActionAt(current);
  next_states.push_back( current.NextState(act_a,C,C) );
  next_states.push_back( current.NextState(act_a,C,D) );
  next_states.push_back( current.NextState(act_a,D,C) );
  next_states.push_back( current.NextState(act_a,D,D) );
}

Graph Strategy::TransitionGraphWithoutPositiveStates() const {
  Graph g(64);
  for( size_t i=0; i<64; i++) {
    FullState fs(i);
    if( fs.RelativePayoff() > 0 ) { continue; }
    std::vector<FullState> next_states;
    NextPossibleFullStates( fs, next_states);
    for( auto next_s: next_states) {
      if( next_s.RelativePayoff() > 0 ) { continue; }
      size_t u = fs.ID();
      size_t v = next_s.ID();
      g.AddLink(u,v);
    }
  }
  return std::move(g);
}

std::ostream &operator<<(std::ostream &os, const Strategy &strategy) {
  os << "actions: ";
  for( auto a : strategy.actions ) {
    os << a;
  }
  os << "\n";
  os << "fullActions: ";
  for( auto a : strategy.fullActions ) {
    os << a;
  }

  return os;
}

std::string Strategy::ToDot() const {
  std::stringstream ss;
  ss << "digraph \"\" {\n";
  for( int i=0; i<64; i++) {
    FullState fs(i);
    int p = fs.RelativePayoff();
    std::string color;
    if( p == 2 ) {
      color = "blue";
    }
    else if( p == 1 ) {
      color = "lightblue";
    }
    else if( p == 0 ) {
      color = "black";
    }
    else if( p == -1 ) {
      color = "orange";
    }
    else if( p == -2 ) {
      color = "red";
    }
    ss << "  " << i << " [ label=\"" << fs << "\"; fontcolor = " << color << " ];\n";
  }
  Graph g = TransitionGraph();
  auto printLink = [&ss](long from, long to) {
    ss << "  " << from << " -> " << to << ";\n";
  };
  g.ForEachLink(printLink);
  ss << "}\n";

  return ss.str();
}

std::string Strategy::ToString() const {
  std::ostringstream oss;
  for( Action act : actions) {
    oss << act;
  }
  return oss.str();
}

bool Strategy::IsDefensible1() const {
  long a[24] = {
      1,3,4,5,6,7,9,11,
      12,13,14,15,33,35,36,37,
      38,39,41,43,44,45,46,47
  };

  const std::set<long> RiskyNodeIDs(a,a+24);
  Graph g = TransitionGraphWithoutPositiveStates();
  std::set<long> nodes = g.TransitionNodes();
  if( nodes.size() < 24 ) { return false; }
  std::set<long> diff;
  std::set_difference( RiskyNodeIDs.begin(), RiskyNodeIDs.end(), nodes.begin(), nodes.end(), std::inserter(diff,diff.end()) );
  return diff.size() == 0;
}

Strategy::int_matrix_t Strategy::Transpose(const Strategy::int_matrix_t &a) const {
  int_matrix_t temp;
  for( size_t i=0; i<a.size(); i++) {
    for( size_t j=0; j<a.size(); j++) {
      temp[i][j] = a[j][i];
    }
  }
  return temp;
}

bool Strategy::IsDefensible() const {

  int_matrix_t A_b; // relative payoff matrix. A_ij = payoff_{Alice} - payoff_{Bob}
  int_matrix_t A_c; // relative payoff matrix. A_ij = payoff_{Alice} - payoff_{Charlie}
  int_matrix_t A1_b, A1_c;
  ConstructA1Matrix(A1_b, A1_c);

  ConstructA1Matrix(A_b, A_c);
  int_matrix_t A1_b_t = Transpose(A_b);
  if( HasNegativeDiagonal(A_b) ) { return false; }
  // if( HasNegativeDiagonal(A_c) ) { return false; } // assuming symmetry between B&C

  for( size_t i=1; i<64; i++) {
    UpdateAMatrix(A_b, A1_b_t);
    if( HasNegativeDiagonal(A_b) ) { return false; }
    /*  assuming symmetry between B&C
    UpdateAMatrix(A_c, A1_c);
    if( HasNegativeDiagonal(A_c) ) { return false; }
    */
  }
  return true;
}

void Strategy::ConstructA1Matrix(Strategy::int_matrix_t &A1_b, Strategy::int_matrix_t &A1_c) const {
  const int INFINITE = 100;
  for( size_t i=0; i<64; i++) {
    FullState state_i(i);
    for( size_t j=0; j<64; j++) { // initialize with a large payoff
      A1_b[i][j] = INFINITE;
      A1_c[i][j] = INFINITE;
    }
    std::vector<FullState> next_states;
    NextPossibleFullStates(state_i, next_states);
    for( auto ns: next_states) {
      size_t j = ns.ID();
      int payoff_against_b = ns.RelativePayoffAgainst(true);
      int payoff_against_c = ns.RelativePayoffAgainst(false);
      A1_b[i][j] = payoff_against_b;
      A1_c[i][j] = payoff_against_c;
    }
  }

}

void Strategy::UpdateAMatrix(Strategy::int_matrix_t &A, const Strategy::int_matrix_t &A1_t) const {
  const int INFINITE = 65536;
  const size_t N = 64;

  int_matrix_t temp;
  for( int i=0; i<N; i++) {
    for( int j=0; j<N; j++) {
      temp[i][j] = INFINITE;
      for( int k=0; k<N; k++) {
        // if( A[i][k] == INFINITE || A1_t[k][j] == INFINITE ) { continue; }
        int aikj = A[i][k] + A1_t[j][k];
        if( aikj < temp[i][j] ) { temp[i][j] = aikj; }
      }
    }
  }

  for( int i=0; i<N; i++) {
    for (int j = 0; j < N; j++) {
      A[i][j] = temp[i][j];
    }
  }
}

bool Strategy::HasNegativeDiagonal(const Strategy::int_matrix_t &A) const {
  for(int i=0; i<64; i++) {
    if( A[i][i] < 0 ) { return true; }
  }
  return false;
}

bool Strategy::IsDistinguishable() const {
  ShortState dc00(D,C,0,0);
  ShortState dd00(D,D,0,0);
  ShortState cc00(C,C,0,0);
  if( ActionAt(dc00) == C && ActionAt(dd00) == C && ActionAt(cc00) == C ) {
    return false;
  }
  return true;
}

FullState FullState::NextState(Action act_a, Action act_b, Action act_c) const {
  return FullState(a_1, act_a, b_1, act_b, c_1, act_c);
}

std::ostream &operator<<(std::ostream &os, const FullState &state) {
  os << state.ID() << '_' << state.a_2 << state.a_1 << state.b_2 << state.b_1 << state.c_2 << state.c_1;
  return os;
}

std::ostream &operator<<(std::ostream &os, const ShortState &state) {
  os << state.ID() << '_' << state.a_2 << state.a_1 << (int)state.bc_2 << (int)state.bc_1;
  return os;
}

