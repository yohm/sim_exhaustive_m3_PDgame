#include "TopologicalEfficiency.hpp"
#include "Strategy.hpp"
#include "TraceNegativeDefensible.hpp"

namespace {

  typedef TopologicalEfficiencyResult_t res_t;

  // Fix action of s at State n, and return the defensibility
  bool FixAction(Strategy& s, long n, Action a) {
    if(s.NumU() > 0) {
      return s.SetActionAndRecalcD(State(n), a);
    }
    else {
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
      TraceNegativeDefensibleResult_t res = TraceNegativeDefensible(s, 4, 64);
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


  // Lc : 0, Ld : 1, Lu : 2
  int JudgeLType(const DirectedGraph& g1, long start_node) {
    // judge if comp is Lc
    if( g1.Reachable(start_node, 0) ) {
      assert( 0 == start_node || !g1.Reachable(0, start_node) );
      return 0;
    }

    // if nodes reachable from comp[0] in g1 contains unfixed nodes (i.e. no outgoing link), it is Lu
    //   to find unfixed nodes, traverse g1 by DFS
    auto ans = FindNodeWithoutOutlink(g1, start_node);
    if( ans.empty() ) {
      // if all nodes are fixed, and cannot reach 0, it is Ld
      return 1;
    }
    else {
      return 2;
    }
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

  void GetLcLdLu(const DirectedGraph& g1, const components_t& candidates, components_t& Lc, components_t& Ld, components_t& Lu) {
    // Lc : component that is reachable to 0 by 1-bit error
    // Ld : component that surely unreachable to 0 by 1-bit error
    // Lu : component that may go 0 or itself or other components

    // judge type of comps
    for(const auto& comp: candidates) {
      int t = JudgeLType(g1, comp[0]);
      if( t == 0 ) {
        Lc.push_back(comp);
      }
      else if( t == 1 ) {
        Ld.push_back(comp);
      }
      else if( t == 2 ) {
        Lu.push_back(comp);
      }
      else {
        throw "must not happen";
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

DirectedGraph ConstructGn1(const DirectedGraph& gn) {
  DirectedGraph gn1 = gn;
  components_t comps = gn.SinkSCCs();
  for(const comp_t& comp: comps) {
    // a node in sinkSCCs must be a fixed node
    // we eliminate the sccs of U/W node
    if( comp.size() == 1 && gn.m_links[comp[0]].size() == 0 ) { continue; }
    for(long from: comp) {
      for(int i=0; i<2; i++) {
        long to = (unsigned long)from^((i==0)?1UL:8UL);
        if( ! gn1.HasLink(from, to) ) {
          gn1.AddLink(from, to);
        }
      }
    }
  }
  return std::move(gn1);
}

std::vector<Strategy> FixStates(const Strategy &str,
                                const DirectedGraph &g1,
                                long start_node,
                                bool is_g2, // true: g2, false: g1
                                uint64_t &n_indefensible) {
  // Traverse g1 from start_node by DFS. If nodes with unfixed actions are found, fix their actions recursively.

  std::vector<long> unfixed_nodes = FindNodeWithoutOutlink(g1, start_node);
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
  std::function<void(const Strategy&,const std::vector<long>&)> dfs = [&ans,&dfs,&n_indefensible,is_g2](const Strategy& s, const std::vector<long>& a) {
    if(a.empty()) {
      ans.push_back(s);
      return;
    }
    long last = a[a.size()-1];
    if(s.ActionAt(last) == U || s.ActionAt(last) == W) {
      for(int i=0; i<2; i++) {
        Strategy _s = s;
        std::vector<long> _a = a;
        _a.pop_back();
        bool d = FixAction(_s, last, (i==0?C:D));
        if(!d) {
          n_indefensible += _s.Size();
          continue;
        }
        const DirectedGraph _g0 = _s.ITG(false);
        const DirectedGraph _g1 = is_g2 ? ConstructGn1(ConstructGn1(_g0)) : ConstructGn1(_g0);
        for(long u : FindNodeWithoutOutlink(_g1, last) ) {
          State sa(u);
          State sb = sa.SwapAB();
          if( _s.ActionAt(sa) == U || _s.ActionAt(sa) == W ) { _a.push_back(sa.ID()); }
          if( _s.ActionAt(sb) == U || _s.ActionAt(sb) == W ) { _a.push_back(sb.ID()); }
        }
        dfs(_s, _a);
      }
    }
    else {
      std::vector<long> _a = a;
      _a.pop_back();
      dfs(s, _a);
    }
  };

  dfs(str, unfixed);
  return ans;
}

void JudgeEfficiencyDFS(const Strategy& s, res_t & f) {
  const DirectedGraph g0 = s.ITG(false);
  const DirectedGraph g1 = ConstructGn1(g0);
  components_t L;
  const components_t sinks = g0.SinkSCCs();
  for(const comp_t& comp: sinks) {
    if(comp.size() == 1 && g0.m_links[comp[0]].size() == 0 ) {
      continue; // eliminate U/W nodes
    }
    L.push_back(comp);
  }

  components_t Lc, Ld, Lu;
  GetLcLdLu(g1, L, Lc, Ld, Lu);
  assert( L.size() == Lc.size() + Ld.size() + Lu.size() );

  // All L belongs to Lc
  if( Ld.empty() && Lu.empty() ) {
    DP("[efficient] Ld and Lu are empty");
    if(s.NumU() > 0) { f.efficient.push_back(s); }
    else {
      f.efficient_and_defensible.push_back(s.ToString());
      f.n_efficient_and_defensible += s.Size();
    }
    return;
  }

  if( Lu.size()>0 ) {
    // Fixing Lu
    DP("Fixing Lu : (# Lu" << Lu.size() << ", Lu[0]:" << Lu[0][0]);
    std::vector<Strategy> v_s2 = FixStates(s, g1, Lu[0][0], false, f.n_rejected);
    for(const Strategy& s2: v_s2) {
      JudgeEfficiencyDFS(s2, f);
    }
    return;
  }
  else {
    DP("Constructing g2");
    // SinkSCC in g1 is all fixed because
    //   - Lc cannot be sinkSCC because it has a link going to 0.
    //   - Ld and its error moves are fixed.
    //   - Lu does not exist.
    // It is enough to judge whether c2 can reach a Ld component
    const DirectedGraph g2 = ConstructGn1(g1);

    for(const comp_t& ld: Ld) {
      if( g2.Reachable(0,ld[0]) ) {
        DP("[rejected] rejected by g2: " << ld[0]);
        // 0->ld[0] occurs with O(e^2). ld[0]->0 occurs with O(e^2) or smaller. cannot be efficient.
        f.n_rejected += s.Size();
        return;
      }
    }

    auto unfixed = FindNodeWithoutOutlink(g2, 0);
    if( unfixed.empty() ) {  // transition from 0 in g2 is all fixed => state-0 is robust against two-bit errors
      bool ok = true;
      for(const comp_t& ld: Ld) {
        if( !g2.Reachable(ld[0],0) ) {
          ok = false;
          break;
        }
      }
      if( ok ) {  // All ld is reachable to 0 in g2
        DP("[efficient] ld->0 reachable while 0->ld is not in g2");
        if(s.NumU() > 0) { f.efficient.push_back(s); }
        else {
          f.efficient_and_defensible.push_back(s.ToString());
          f.n_efficient_and_defensible += s.Size(); }
      }
      else {
        // we cannot judge the efficiency without considering higher order
        DP("[pending] cannot judge");
        f.pending.push_back(s);
      }
      return;
    }
    else {
      if(s.NumU() > 0) { // when defensibility is not assured, check defensibility first
        f.pending.push_back(s);
        return;
      }
      std::vector<Strategy> v_s2 = FixStates(s, g2, 0, true, f.n_rejected);
      for(const Strategy& s2: v_s2) {
        JudgeEfficiencyDFS(s2, f);
      }
      return;
    }
  }
}

  void JudgeEfficiencyDFS2(const Strategy& strategy, res_t & f) {
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
          assert( errors > 1 ); // when errors=1, this cannot happen
          f.n_rejected += strategy.Size();
          return;
        }
      }

      for(int l: scc_nodes) {
        if( !FindNodeWithoutOutlink(gn, l).empty() ) {
          DP("Fixing nodes starting from :" << l);
          std::vector<Strategy> strategies2 = FixStates(strategy, gn, l, f.n_rejected);
          for(const Strategy& s: strategies2) {
            JudgeEfficiencyDFS2(s, f);
          }
          return;
        }
      }
      DP("all nodes that are reachable from scc are fixed.");
    }
  }

}

TopologicalEfficiencyResult_t CheckTopologicalEfficiency(Strategy& str) {
  TopologicalEfficiencyResult_t res;
  if( !str.IsDefensible() ) {
    res.n_rejected += str.Size();
    return std::move(res);
  }
  std::vector<Strategy> assigned = FixL0(str, res.n_rejected);
#ifndef NDEBUG
  TopologicalEfficiencyResult_t res0;
  res0.n_rejected = res.n_rejected;
#endif

  for(const Strategy& s: assigned) {
    JudgeEfficiencyDFS2(s, res);
#ifndef NDEBUG
    uint64_t offset = res0.NumTotal();
    JudgeEfficiencyDFS(s, res0);
    res.PrintStrategies(std::cerr);
    res0.PrintStrategies(std::cerr);
    assert( res.NumTotal() == s.Size() + offset );
    assert( res0.NumTotal() == s.Size() + offset );
    assert( res.NumTotal() == res0.NumTotal() );
    for(const auto& s: res.efficient) {
      TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s,64,64);
      res.n_efficient_and_defensible += res_d.NumPassed();
      res.n_rejected += res_d.n_rejected;
    }
    res.efficient.clear();
    for(const auto& s: res0.efficient) {
      TraceNegativeDefensibleResult_t res_d = TraceNegativeDefensible(s,64,64);
      res0.n_efficient_and_defensible += res_d.NumPassed();
      res0.n_rejected += res_d.n_rejected;
    }
    res0.efficient.clear();
    assert( res.n_efficient_and_defensible == res0.n_efficient_and_defensible );
    // assert( res.NumEfficient() == res2.NumEfficient() );
    assert( res.n_rejected == res0.n_rejected );
    assert( res.NumTotal() == res0.NumTotal() );
#endif
    std::cout << s.ToString() << " " << res.efficient.size() << ' ' << res.n_rejected << std::endl;
  }
  return std::move(res);
}

