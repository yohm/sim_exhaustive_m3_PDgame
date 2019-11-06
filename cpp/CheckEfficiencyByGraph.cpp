#include "CheckEfficiencyByGraph.hpp"
#include "Strategy.hpp"
#include "TraceGSNegatives.hpp"

namespace {

  typedef CheckEfficiencyResult_t res_t;

  // Fix action of s at State n, and return the defensibility
  bool FixAction(Strategy& s, long n, Action a) {
    if(s.NumU() > 0) {
      return s.SetActionAndRecalcD(State(n), a);
    }
    else {  // defensibility is already assured in the previous steps
      s.SetAction(State(n), a);
      return true;
    }
  }

  void AssignActions(const Strategy& s, std::set<long> to_be_fixed, std::vector<Strategy>& ans, uint64_t& n_indefensible) {
    if( to_be_fixed.empty() ) {
      ans.push_back(s);
      return;
    }

    auto it = to_be_fixed.begin();
    long n = *it;
    to_be_fixed.erase(it);
    for(int i=0; i<2; i++) {
      Strategy _s = s;
      bool d = FixAction(_s, n, (i==0?C:D));
      if(!d) {
        n_indefensible += _s.Size();
        continue;
      }
      AssignActions(_s, to_be_fixed, ans, n_indefensible);
    }
  }

  std::vector<Strategy> FixL0(const Strategy& str, uint64_t& n_indefensible) {
    DirectedGraph g = str.ITG(true);
    components_t comps = g.NonTransitionComponents();

    // fix l0
    std::set<long> to_be_fixed;
    for(const auto& comp: comps) {
      for(long n: comp) {
        State sa(n);
        State sb = sa.SwapAB();
        Action act_a = str.ActionAt(sa);
        Action act_b = str.ActionAt(sb);
        if( act_a == U || act_a == W ) { to_be_fixed.insert(sa.ID()); }
        if( act_b == U || act_b == W ) { to_be_fixed.insert(sb.ID()); }
      }
    }
    // std::cerr << "to_be_fixed in FixL0: " << to_be_fixed.size() << std::endl;

    if( str.NumU() > 0 && to_be_fixed.size() > 8 ) {
      Strategy s = str;
      TraceGSNegativesResult_t res = TraceGSNegatives(s, 4, 64);
      n_indefensible += res.n_rejected;
      std::vector<Strategy> ans;
      for(const Strategy& x: res.passed) {
        std::vector<Strategy> _v = FixL0(x, n_indefensible);
        ans.insert(ans.end(), _v.begin(), _v.end());
      }
      return std::move(ans);
    }
    else {
      std::vector<Strategy> ans;
      AssignActions(str, to_be_fixed, ans, n_indefensible);
      return std::move(ans);
    }
  }

  // find nodes having no outgoing link by BFS starting from ini
  std::vector<long> FindNodeWithoutOutlink(const DirectedGraph& g, long ini) {
    std::vector<long> ans;
    std::stack<long> stk;
    std::vector<int> visited(g.m_num_nodes, 0);
    stk.push(ini);
    while( stk.size() > 0 ) {
      long i = stk.top();
      stk.pop();
      if( visited[i] > 0 ) continue;
      visited[i] = 1;
      if( g.m_links[i].size() == 0 ) {
        ans.push_back(i);
      }
      for(long j: g.m_links[i]) {
        if( visited[j] ) continue;
        stk.push(j);
      }
    }
    return ans;
  }

  void UpdateGn(DirectedGraph& g) {
    components_t sinks = g.SinkSCCs();

    for(const comp_t& sink: sinks) {
      if( sink.size() == 1 && g.m_links[sink[0]].size() == 0 ) { continue; } // skip unfixed nodes
      for(long from: sink) {
        assert( g.m_links[from].size() > 0 );  // A node sink SCC must be a fixed node.
        for(int i=0; i<2; i++) {
          long to = (unsigned long)from^((i==0)?1UL:8UL);
          if( !g.HasLink(from, to) ) { g.AddLink(from, to); }
        }
      }
    }
  }

  std::vector<Strategy> FixStates(const Strategy &str,
                                  const DirectedGraph &gn,
                                  long start_node,
                                  uint64_t &n_indefensible) {
    // Traverse gn from start_node by DFS. If nodes with unfixed actions are found, fix their actions recursively.

    std::vector<long> unfixed_nodes = FindNodeWithoutOutlink(gn, start_node);
    std::vector<long> unfixed;
    for(long n: unfixed_nodes) {
      State sa(n);
      State sb = sa.SwapAB();
      assert( str.ActionAt(sa) == U || str.ActionAt(sa) == W || str.ActionAt(sb) == U || str.ActionAt(sb) == W );
      if( str.ActionAt(sa) == U || str.ActionAt(sa) == W ) { unfixed.push_back(sa.ID()); }
      if( str.ActionAt(sb) == U || str.ActionAt(sb) == W ) { unfixed.push_back(sb.ID()); }
    }
    std::sort( unfixed.begin(), unfixed.end() );
    unfixed.erase( std::unique(unfixed.begin(), unfixed.end()), unfixed.end() );

    // Recursively construct strategies
    std::vector<Strategy> ans;
    std::function<void(const Strategy&,const DirectedGraph&,const std::vector<long>&)> dfs = [&ans,&dfs,&n_indefensible](const Strategy& s, const DirectedGraph& g, const std::vector<long>& a) {
      if(a.empty()) {
        ans.push_back(s);
        return;
      }
      long last = a[a.size()-1];
      if(s.ActionAt(last) == U || s.ActionAt(last) == W) {
        for(int i=0; i<2; i++) {
          Strategy _s = s;
          std::vector<long> _a = a;
          DirectedGraph _g = g;
          _a.pop_back();
          bool d = FixAction(_s, last, (i==0?C:D));
          if(!d) {
            n_indefensible += _s.Size();
            continue;
          }
          { // update _g
            State current(last);
            Action act_b = _s.ActionAt(current.SwapAB());
            if( act_b == C || act_b == D ) {
              State next_state = current.NextState( _s.ActionAt(current), _s.ActionAt(current.SwapAB()) );
              if( !_g.HasLink(current.ID(), next_state.ID()) ) { _g.AddLink(current.ID(), next_state.ID()); }
              if( !_g.HasLink(current.SwapAB().ID(), next_state.SwapAB().ID()) ) { _g.AddLink(current.SwapAB().ID(), next_state.SwapAB().ID()); }
            }
          }
          for(long u : FindNodeWithoutOutlink(_g, last) ) {
            State sa(u);
            State sb = sa.SwapAB();
            if( _s.ActionAt(sa) == U || _s.ActionAt(sa) == W ) { _a.push_back(sa.ID()); }
            if( _s.ActionAt(sb) == U || _s.ActionAt(sb) == W ) { _a.push_back(sb.ID()); }
          }
          dfs(_s, _g, _a);
        }
      }
      else {
        std::vector<long> _a = a;
        _a.pop_back();
        dfs(s, g, _a);
      }
    };

    dfs(str, gn, unfixed);
    return ans;
  }

  void JudgeEfficiencyDFS(const Strategy &strategy, res_t &f) {
    const DirectedGraph g0 = strategy.ITG(false);
    components_t L;
    const components_t sccs = g0.NonTransitionComponents();
    std::vector<int> scc_nodes, unjudged;
    for(const comp_t& comp: sccs) {
      assert( g0.m_links[comp[0]].size() > 0 );
      if( comp.size() == 1 && comp[0] == 0 ) { scc_nodes.push_back(comp[0]); }
      else { scc_nodes.push_back(comp[0]); unjudged.push_back(comp[0]); }
    }
    std::sort( scc_nodes.begin(), scc_nodes.end() );
    std::sort( unjudged.begin(), unjudged.end() );

    DirectedGraph gn = g0;
    long errors = 0;

    // polish unjudged based on gn
    auto FilterUnjudged = [&unjudged](const DirectedGraph& gn) {
      std::vector<int> to_be_removed;
      for(int l: unjudged) {
        if( gn.Reachable(l,0) ) {
          DP("  path " << l << " -> 0 exists.");
          to_be_removed.push_back(l);
        }
      }
      for(int n: to_be_removed) {
        auto it = std::find(unjudged.begin(), unjudged.end(), n);
        unjudged.erase(it);
      }
    };

    while(true) {
      // check path from l->0
      FilterUnjudged(gn);
      if( unjudged.empty() ) {
        DP("[efficient] unjudged is empty at errors=" << errors);
        if( strategy.NumU() > 0 ) { f.efficient.push_back(strategy); }
        else {
          f.efficient_and_defensible.push_back(strategy.ToString());
          f.n_efficient_and_defensible += strategy.Size();
        }
        return;
      }

      // At this moment, all SCCs in Gn is fixed.
      UpdateGn(gn);
      errors++;

      // check path from 0->l
      for(int l : unjudged) {
        if( gn.Reachable(0, l) ) {  // inefficient
          DP("[inefficient] path 0->" << l << "exists in g_" << errors);
          assert( errors > 1 ); // when errors=1, this cannot happen because one-bit tolerance is checked in the previous steps
          f.n_rejected += strategy.Size();
          return;
        }
      }

      for(int l: scc_nodes) {
        if( !FindNodeWithoutOutlink(gn, l).empty() ) {
          DP("Fixing nodes starting from :" << l);
          std::vector<Strategy> strategies2 = FixStates(strategy, gn, l, f.n_rejected);
          for(const Strategy& s: strategies2) {
            JudgeEfficiencyDFS(s, f);
          }
          return;
        }
      }
      DP("all nodes that are reachable from scc are fixed.");
    }
  }

}

CheckEfficiencyResult_t CheckEfficiencyByGraph(Strategy &str) {
  CheckEfficiencyResult_t res;
  if( !str.IsDefensible() ) {
    res.n_rejected += str.Size();
    return std::move(res);
  }
  std::vector<Strategy> assigned = FixL0(str, res.n_rejected);
  for(const Strategy& s: assigned) {
    JudgeEfficiencyDFS(s, res);
  }
  return std::move(res);
}

