#include <iostream>
#include <set>
#include <map>
#include "Strategy.hpp"


Strategy::Strategy(const std::array<Action,64>& acts): actions(acts), d_matrix_ready(false) {}

Strategy::Strategy(const char acts[64]) : d_matrix_ready(false) {
  for( size_t i=0; i<64; i++) {
    actions[i] = C2A(acts[i]);
  }
}

std::vector<State> Strategy::NextPossibleStates(State current) const {
  std::vector<State> next_states;
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
  return std::move(next_states);
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

inline int8_t MIN(int8_t a, int8_t b) { return (a<b)?a:b; }

bool Strategy::IsDefensible() {
  d_matrix_ready = false;

  // construct adjacency matrix
  const size_t N = 64;
  const int INF = 32; // 32 is large enough since the path length is between -16 to 16.
  for(size_t i=0; i<N; i++) {
    for(size_t j=0; j<N; j++) {
      m_d[i][j] = INF;
    }
  }

  for(size_t i=0; i<N; i++) {
    if(actions[i]==U) continue;
    State si(i);
    std::vector<State> sjs = NextPossibleStates(si);
    for( auto sj: sjs) {
      size_t j = sj.ID();
      m_d[i][j] = si.RelativePayoff();
    }
    if(m_d[i][i] < 0) { return false; }
  }

  for(size_t k=0; k<N; k++) {
    if(actions[k]==U) continue; // path i-k-j cannot improve m_d[i][j] since k does not have an out link
    for(size_t i=0; i<N; i++) {
      if(actions[i]==U) continue; // m_d[i][j] is not updated since it is always INFINITY when i does not have an out-link
      for(size_t j=0; j<N; j++) {
        m_d[i][j] = MIN(m_d[i][j], m_d[i][k]+m_d[k][j]);
      }
      if(m_d[i][i] < 0) { return false; }
    }
  }
  d_matrix_ready = true;
  return true;
}

// set action[s]=a, and recalculate `m_d`. If not defensible, return false.
bool Strategy::SetActionAndRecalcD(const State &sk, Action a) {
  assert(actions[sk.ID()]==U);

  assert(d_matrix_ready);

  d_matrix_ready = false;

  size_t k = sk.ID();
  actions[k] = a;

  // calculate m_d[k][j], by the introduction of out-links from k, m_d[k][j] may change.
  std::vector<State> sis = NextPossibleStates(sk);
  for( auto si: sis) {
    size_t i = si.ID();
    m_d[k][i] = MIN(m_d[k][i], sk.RelativePayoff() );
    if(actions[i]==U) continue;  // we don't have to check since m_d[i][j]=INFINITY
    for(size_t j=0; j<m_d[i].size(); j++) {
      m_d[k][j] = MIN( m_d[k][j], m_d[k][i]+m_d[i][j] );
    }
  }
  if(m_d[k][k] < 0) { return false; }

  // recalculate m_d[i][j] since it may change by the introduction of k
  for(size_t i=0; i<m_d.size(); i++) {
    if(actions[i]==U) continue; // m_d[i][j] is not updated since it is always INFINITY when i does not have an out-link
    for(size_t j=0; j<m_d[i].size(); j++) {
      m_d[i][j] = MIN(m_d[i][j], m_d[i][k]+m_d[k][j]);
    }
    if(m_d[i][i] < 0) { return false; }
  }

  d_matrix_ready = true;
  return true;
}
std::vector<State> Strategy::DanglingStates() const {
  std::set<State> ans;
  for(int i=0; i<64; i++) {
    if(actions[i] != U) {
      std::vector<State> sjs = NextPossibleStates(State(i));
      for(auto sj: sjs) {
        if( ActionAt(sj) == U ) {
          ans.insert(sj);
        }
      }
    }
  }
  std::vector<State> ret(ans.begin(), ans.end());
  return std::move(ret);
}

std::vector<State> Strategy::NegativeDanglingStates() const {
  if(!d_matrix_ready) {  // calculate d_matrix
    throw "must not happen";  // `IsDefensible` must be called beforehand
  }
  std::multimap<int,int> m;  // to sort_by the riskiness, min_{i} m_d[j][i], multimap is used, multimap is used

  std::vector<int> i_candidates;
  i_candidates.reserve(16);  // at most 16
  for( int i=0; i<64; i++) {
    if( (i&9)==1 && actions[i] != U) {
      bool no_incoming = true;
      for( auto prev: State(i).PossiblePrevStates() ) {
        if( ActionAt(prev) == U || ActionAt(prev) == State(i).a_1 ) no_incoming = false;
      }
      if( no_incoming ) {
        // std::cerr << "REJECTED BY NO INCOMING LINK CONDITION" << std::endl;
        continue; }  // It's sure that i has no incoming link.
      i_candidates.push_back(i);
    }
  }

  for(int j=0; j<64; j++) {
    if(actions[j] == U) {
      int min = 0;
      for(int i : i_candidates) {
        if( m_d[i][j] < min ) { min = m_d[i][j]; }
      }
      if(min < 0) {
        m.insert( std::make_pair(min,j) );
      }
    }
  }

  std::vector<State> ans;
  for( auto p : m ) {
    ans.emplace_back( p.second );
  }

  return std::move(ans);
}
std::array<int, 64> Strategy::DestsOfITG() const {
  std::array<int, 64> dests = {};
  std::array<bool, 64> fixed = {false};

  for(int i=0; i<64; i++) {
    std::array<bool,64> visited = { false }; // initialize by false
    visited[i] = true;
    State init(i);
    int next = NextITGState(init);
    while(next >= 0) {
      if(visited[next] || fixed[next]) { break; }
      visited[next] = true;
      next = NextITGState(State(next));
    }
    int d = next;
    if(next >= 0) {
      d = fixed[next] ? dests[next] : next;
    }
    for(uint64_t j=0; j<64; j++) {
      if(visited[j]) {
        dests[j] = d;
        fixed[j] = true;
      }
    }
  }
  return dests;
}

int Strategy::NextITGState(const State &s) const {
  Action move_a = ActionAt(s);
  Action move_b = ActionAt(s.SwapAB());
  if( (move_a == C || move_a == D) && (move_b == C || move_b == D) ){
    return s.NextState(move_a, move_b).ID();
  }
  return -1;
}

std::array<double, 64> Strategy::StationaryState(double e) const {
  assert( NumFixed() == 64 );
  Eigen::Matrix<double,65,64> A;

  for(int i=0; i<64; i++) {
    const State si(i);
    for(int j=0; j<64; j++) {
      // calculate transition probability from j to i
      const State sj(j);
      State next = NextITGState(sj);
      int d = next.NumDiffInT1(si);
      if( d < 0 ) {
        A(i,j) = 0.0;
      }
      else if( d == 0 ) {
        A(i,j) = (1.0-e)*(1.0-e);
      }
      else if( d == 1 ) {
        A(i,j) = (1.0-e)*e;
      }
      else if( d == 2 ) {
        A(i,j) = e*e;
      }
      else {
        assert(false);
      }
    }
    A(i,i) = A(i,i) - 1.0;  // subtract unit matrix
  }
  for(int i=0; i<64; i++) { A(64,i) = 1.0; }  // normalization condition

  Eigen::VectorXd b(65);
  for(int i=0; i<64; i++) { b(i) = 0.0;}
  b(64) = 1.0;

  Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

  std::array<double,64> ans = {0};
  for(int i=0; i<64; i++) { ans[i] = x(i); }
  /*
  std::cerr << "ans[0] , ans[63] : " << ans[0] << ' ' << ans[63] << std::endl;
  if(ans[0] > 0.95) {
    assert(true);
  }
   */
  return ans;
}
bool Strategy::CannotBeEfficient() const {
  std::array<int,64> dests = DestsOfITG();
  if( dests[State("ccdccc").ID()] == -1 || dests[State("cccccd").ID()] == -1 ) { return false; } // cannot judge efficiency
  if( dests[State("ccdccc").ID()] > 0 || dests[State("cccccd").ID()] > 0 ) { return true; } // one-bit tolerance of State(0) is absent. cannot be efficient.

  auto TraceITG = [this, &dests](const State& s) {
    std::set<int> histo;
    histo.insert( s.ID() );
    int n = this->NextITGState(s);
    while( histo.find(n) == histo.end() ) {
      histo.insert(n);
      if( n < 0 ) { break; }
      n = this->NextITGState(n);
    }
    return std::move(histo);
  };

  std::map<int,bool> memo;
  auto CannotReturnTo0By1BitError = [this, &dests, &TraceITG, &memo](const State& s) {
    if( memo.find(s.ID()) != memo.end() ) {
      return memo.at(s.ID());
    }
    // true : It is sure that it cannot reach state0 by 1-bit error
    std::set<int> set0 = TraceITG(s);  // nodes accessible by 0-bit error
    if( set0.find(-1) != set0.end() ) {
      memo[s.ID()] = false;
      return false;  // unfixed node exists.
    }
    std::set<int> set1;
    for(int n0: set0) {
      for(State s1 : State(n0).NoisedStates() ) {
        set1.insert(s1.ID());
      }
    }
    std::set<int> set1_dests;
    for(int n1: set1) {
      set1_dests.insert( dests[n1] );
    }

    if( set1_dests.find(-1) != set1_dests.end() ) {
      memo[s.ID()] = false;
      return false;  // unfixed node exists
    }
    if( set1_dests.find(0) == set1_dests.end() ) {
      // no path returning to 0
      // It is sure that the state cannot return to "cccccc" by 1-bit error
      memo[s.ID()] = true;
      return true;
    }
    memo[s.ID()] = false;
    return false;
  };

  std::set<int> c1s = {0}; // "cccccc" is included
  { // populate c1s
    auto c1_a = TraceITG( State("ccdccc") );
    c1s.insert(c1_a.begin(), c1_a.end());
    auto c1_b = TraceITG( State("cccccd") );
    c1s.insert(c1_b.begin(), c1_b.end());
  }

  for(int c1: c1s) {
    if( c1 < 0 ) { continue; }
    for(State c2: State(c1).NoisedStates() ) {
      int d = dests[ c2.ID() ];
      if( d <= 0 ) { continue; }  // not judgeable if 0 or -1
      if( CannotReturnTo0By1BitError(State(d)) ) {
        return true;
      }
    }
  }
  return false; // cannot judge whether it is efficient or not
}
DirectedGraph Strategy::ITG() const {
  DirectedGraph g(64);
  for(int i=0; i<64; i++) {
    State sa(i);
    State sb = sa.SwapAB();
    int j = sb.ID();
    std::vector<Action> acts_a, acts_b;
    if( actions[i] == U || actions[i] == W ) { acts_a.push_back(C); acts_a.push_back(D); }
    else { acts_a.push_back(actions[i]); }
    if( actions[j] == U || actions[j] == W ) { acts_b.push_back(C); acts_b.push_back(D); }
    else { acts_b.push_back(actions[j]); }

    for(Action act_a: acts_a) {
      for(Action act_b: acts_b) {
        int n = sa.NextState(act_a, act_b).ID();
        g.AddLink(i, n);
      }
    }
  }

  return std::move(g);
}

