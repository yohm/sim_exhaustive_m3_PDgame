//
// Created by Yohsuke Muraes on 2017/06/05.
//

#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <functional>

typedef std::vector< std::vector<long> > components_t;

class Graph {
public:
  Graph(size_t num_nodes);

  void AddLink(long from, long to);
  friend std::ostream &operator<<(std::ostream &os, const Graph &graph);
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
    ComponentFinder(const Graph &m_g);
    void SCCs( components_t& components );
  private:
    const Graph& m_g;
    long m_t;
    std::vector<long> desc;
    std::vector<long> low;
    std::stack<long> stack;
    std::vector<bool> on_stack;

    void StrongConnect( long v, components_t& components);

  };
};


#endif //GRAPH_HPP

