//
// Created by Yohsuke Murase on 2017/06/26.
//

#include <iostream>
#include <set>
#include "StrategyM3.hpp"


FullStateM3 FullStateM3::NextState(Action act_a, Action act_b, Action act_c) const {
  return FullStateM3(a_2,a_1,act_a,b_2,b_1,act_b,c_2,c_1,act_c);
}

std::ostream &operator<<(std::ostream &os, const FullStateM3 &state) {
  os << state.ID() << '_'
     << A2C(state.a_3) << A2C(state.a_2) << A2C(state.a_1)
     << '-' << A2C(state.b_3) << A2C(state.b_2) << A2C(state.b_1)
     << '-' << A2C(state.c_3) << A2C(state.c_2) << A2C(state.c_1);
  return os;
}


StrategyM3::StrategyM3(const char *acts) {
  for( size_t i=0; i<512; i++) {
    if( acts[i] != 'c' && acts[i] != 'd' ) { throw "invalid input"; }
    actions[i] = (acts[i] == 'c' ? C : D);
  }
}

Graph StrategyM3::TransitionGraph() const {
  Graph g(512);
  for( size_t i=0; i<512; i++) {
    FullStateM3 fs(i);
    std::vector<FullStateM3> next_states = NextPossibleFullStates(fs);
    for( auto next_s: next_states) {
      size_t u = fs.ID();
      size_t v = next_s.ID();
      g.AddLink(u,v);
    }
  }
  return std::move(g);
}

std::vector<FullStateM3> StrategyM3::NextPossibleFullStates(FullStateM3 current) const {
  std::vector<FullStateM3> next_states;
  Action act_a = ActionAt(current);
  next_states.push_back( current.NextState(act_a,C,C) );
  next_states.push_back( current.NextState(act_a,C,D) );
  next_states.push_back( current.NextState(act_a,D,C) );
  next_states.push_back( current.NextState(act_a,D,D) );
  return std::move(next_states);
}

std::ostream &operator<<(std::ostream &os, const StrategyM3 &strategy) {
  for( size_t i=0; i<strategy.actions.size(); i++) {
    Action act = strategy.actions[i];
    FullStateM3 stat(i);
    os << A2C(act) << '|' << stat << "\t";
    if( i%10 == 9 ) { os << "\n"; }
  }
  os << "\n";

  return os;
}

std::string StrategyM3::ToDot() const {
  std::stringstream ss;
  ss << "digraph \"\" {\n";
  for( int i=0; i<512; i++) {
    FullStateM3 fs(i);
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

std::string StrategyM3::ToString() const {
  std::ostringstream oss;
  for( Action act : actions) {
    if(act == Action::C) { oss << 'c';}
    else if(act == Action::D) { oss << 'd'; }
    else { throw "must not happen"; }
  }
  return oss.str();
}

bool StrategyM3::IsDefensible() const {

  a_matrix_t A_b; // relative payoff matrix. A_ij = payoff_{Alice} - payoff_{Bob}
  a_matrix_t A_c; // relative payoff matrix. A_ij = payoff_{Alice} - payoff_{Charlie}

  ConstructA1Matrix(A_b, A_c);
  a_matrix_t A1_b_t = Transpose(A_b);
  // a_matrix_t A1_c_t = Transpose(A_c);
  if( HasNegativeDiagonal(A_b) ) { return false; }
  // if( HasNegativeDiagonal(A_c) ) { return false; } // assuming symmetry

  for( size_t i=1; i<512; i++) {
    UpdateAMatrix(A_b, A1_b_t);
    if( HasNegativeDiagonal(A_b) ) { return false; }
    // UpdateAMatrix(A_c, A1_c_t);
    // if( HasNegativeDiagonal(A_c) ) { return false; }
  }

  //for( auto x: A_b[0] ) { std::cerr << x << ' '; }
  //std::cerr << std::endl;
  return true;
}

void StrategyM3::ConstructA1Matrix(StrategyM3::a_matrix_t &A1_b, StrategyM3::a_matrix_t &A1_c) const {
  const int INFINITE = 65536;
  for( size_t i=0; i<512; i++) {
    FullStateM3 state_i(i);
    for( size_t j=0; j<512; j++) { // initialize with a large payoff
      A1_b[i][j] = INFINITE;
      A1_c[i][j] = INFINITE;
    }
    std::vector<FullStateM3> next_states = NextPossibleFullStates(state_i);
    for( auto ns: next_states) {
      size_t j = ns.ID();
      int payoff_against_b = ns.RelativePayoffAgainst(true);
      int payoff_against_c = ns.RelativePayoffAgainst(false);
      A1_b[i][j] = payoff_against_b;
      A1_c[i][j] = payoff_against_c;
    }
  }
}

StrategyM3::a_matrix_t StrategyM3::Transpose(const StrategyM3::a_matrix_t &a) const {
  a_matrix_t temp;
  for( size_t i=0; i<a.size(); i++) {
    for( size_t j=0; j<a.size(); j++) {
      temp[i][j] = a[j][i];
    }
  }
  return temp;
}

void StrategyM3::UpdateAMatrix(StrategyM3::a_matrix_t &A, const StrategyM3::a_matrix_t &A1_t) const {
  const int INFINITE = 65536;
  const size_t N = 512;

  a_matrix_t temp;
  for( int i=0; i<N; i++) {
    for( int j=0; j<N; j++) {
      temp[i][j] = INFINITE;
      for( int k=0; k<N; k++) {
        //if( A[i][k] == INFINITE || A1[k][j] == INFINITE ) { continue; }
        int aikj = A[i][k] + A1_t[j][k];
        if( aikj < temp[i][j] ) { temp[i][j] = aikj; }
      }
    }
  }

  for( int i=0; i<N; i++) {
    for (int j = 0; j <N; j++) {
      A[i][j] = temp[i][j];
    }
  }
}

bool StrategyM3::HasNegativeDiagonal(const StrategyM3::a_matrix_t &A) const {
  for(int i=0; i<A.size(); i++) {
    if( A[i][i] < 0 ) { return true; }
  }
  return false;
}

bool StrategyM3::IsDistinguishable() const {
  throw "not implemented yet";
  return true;
}

StrategyM3 StrategyM3::AllC() {
  const char* allc =
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
  return StrategyM3(allc);
}

