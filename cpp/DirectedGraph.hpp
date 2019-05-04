#ifndef DIRECTED_GRAPH_HPP
#define DIRECTED_GRAPH_HPP

#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <functional>

typedef std::vector< std::vector<long> > components_t;

class DirectedGraph {
public:
  DirectedGraph(size_t num_nodes);

  void AddLink(long from, long to);
  friend std::ostream &operator<<(std::ostream &os, const DirectedGraph &graph);
  void SCCs(components_t& components) const {
    ComponentFinder cf(*this);
    cf.SCCs(components);
  }
  std::set<long> TransitionNodes() const;
  template <class T>
  void ForEachLink( const T& f) const {
    for( long i=0; i<m_num_nodes; i++) {
      for( long j: m_links[i]) {
        f(i,j);
      }
    }
  }

  const size_t m_num_nodes;
  std::vector<std::vector<long> > m_links;
private:
  bool HasSelfLoop(long n) const; // return true if node n has a self-loop

  class ComponentFinder {
  public:
    ComponentFinder(const DirectedGraph &m_g);
    void SCCs( components_t& components );
  private:
    const DirectedGraph& m_g;
    long m_t;
    std::vector<long> desc;
    std::vector<long> low;
    std::stack<long> stack;
    std::vector<bool> on_stack;

    void StrongConnect( long v, components_t& components);

  };
};

DirectedGraph::DirectedGraph(size_t num_nodes) : m_num_nodes(num_nodes) {
  m_links.resize(m_num_nodes);
}

void DirectedGraph::AddLink(long from, long to) {
  m_links[from].push_back(to);
}

std::ostream &operator<<(std::ostream &os, const DirectedGraph &graph) {
  os << "m_num_nodes: " << graph.m_num_nodes << "\n";
  os << "m_links: \n";
  for( long from=0; from < graph.m_num_nodes; from++) {
    for( auto to : graph.m_links[from] ) {
      os << "  " << from << " " << to << "\n";
    }
  }
  return os;
}

std::set<long> DirectedGraph::TransitionNodes() const {
  std::set<long> transition_nodes;

  components_t components;
  SCCs(components);
  for( const std::vector<long>& component: components) {
    if( component.size() == 1 ) {
      long n = component[0];
      if( !HasSelfLoop(n) ) { transition_nodes.insert(n); }
    }
  }

  return std::move(transition_nodes);
}

DirectedGraph::ComponentFinder::ComponentFinder(const DirectedGraph &m_g) : m_g(m_g) {
  size_t n = m_g.m_num_nodes;
  m_t = 0;
  desc.resize(n,-1);
  low.resize(n,-1);
  on_stack.resize(n,false);
}

void DirectedGraph::ComponentFinder::SCCs( components_t& components ) {
  components.clear();

  for( long v=0; v < m_g.m_num_nodes; v++ ) {
    if( desc[v] < 0 ) {
      StrongConnect(v, components);
    }
  }
}

void DirectedGraph::ComponentFinder::StrongConnect(long v, components_t &components) {
  desc[v] = m_t;
  low[v] = m_t;
  m_t++;

  stack.push(v);
  on_stack[v] = true;

  for( long w : m_g.m_links.at(v) ) {
    if( desc[w] < 0 ) {
      StrongConnect(w, components);
      if( low[w] < low[v] ) { low[v] = low[w]; }
    }
    else if( on_stack[w] ) {
      if( desc[w] < low[v] ) { low[v] = desc[w]; }
    }
  }

  std::vector<long> comp;
  if( low[v] == desc[v] ) {
    while(true) {
      long w = stack.top();
      stack.pop();
      on_stack[w] = false;
      comp.push_back(w);
      if( v==w ) { break; }
    }
    components.push_back(comp);
  }
}

bool DirectedGraph::HasSelfLoop(long n) const {
  bool b = false;
  for( long j: m_links[n]) {
    if( j == n ) {
      b = true;
    }
  }
  return b;
}

#endif //DIRECTED_GRAPH_HPP