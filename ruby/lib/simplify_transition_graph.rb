require_relative 'strategy'
require_relative 'graph'

module DFAminimize

  class UnionFind
    def initialize(n)
      @par  = Array.new(n) {|i| i}
    end

    def root(i)
      if @par[i] != i
        r = root(@par[i])
        @par[i] = r
      end
      @par[i]
    end

    def roots
      to_h.keys.sort
    end

    def merge(i, j)
      ri = root(i)
      rj = root(j)
      return false if ri == rj  # already merged
      ri,rj = rj,ri if ri > rj
      @par[rj] = ri
    end

    def to_h
      @par.size.times.group_by {|i| root(i) }
    end
  end

  def self.equivalent(dest_i, dest_j, h2a, uf)
    # both action & next state must be identical
    di = dest_i.map {|x| [h2a.call(x), uf.root(x)] }.sort
    dj = dest_j.map {|x| [h2a.call(x), uf.root(x)] }.sort
    di == dj
  end

  def self.minimize_DFA(str)
    g = str.transition_graph
    h2a = lambda {|x| [8,1].map{|m| ((x&m)==m)?'d':'c'}.join }
    uf_0 = UnionFind.new(g.n)
    # initial grouping by the action c/d
    g.n.times do |i|
      act = h2a.call( g.links[i][0] )[0]
      if act == 'c'
        uf_0.merge(i, 0)
      else
        uf_0.merge(i, g.n-1)
      end
    end

    loop do
      uf_0_h = uf_0.to_h
      uf = UnionFind.new(g.n)
      uf_0_h.each do |r,s|  # refinint a set in uf_0
        s.combination(2).each do |i,j|
          if equivalent(g.links[i], g.links[j], h2a, uf_0)
            uf.merge(i,j)
          end
        end
      end
      break if uf.to_h == uf_0_h
      uf_0 = uf
    end

    g2 = DirectedGraph.new(g.n)
    g.for_each_link do |i,j|
      ri = uf_0.root(i)
      rj = uf_0.root(j)
      g2.add_link(ri,rj)
    end
    g2.remove_duplicate_links!

    return uf_0, g2
  end
end

if __FILE__ == $0 and ARGV.size == 1
  DEBUG = true

  s = ARGV[0]
  raise "unsupported input format" unless s.length == 64

  s = s.gsub('*', 'c')
  str = Strategy.make_from_str(s)
  pp str
  puts "defensible?      : #{str.defensible?}"
  puts "efficient?       : #{str.efficient?}"
  puts "distinguishable? : #{str.distinguishable?}"

  # transition probs in g_SS
  File.open("g_ss.dot", 'w') do |io|
    io.puts str.transition_graph_with_self.to_dot
  end
  $stderr.puts "g_ss.dot was written"
  def calc_transition_probs(str)
    g = str.transition_graph_with_self
    sccs = g.terminanl_components
    pp sccs
    transition_probs = {}
    sccs.map {|c| c.sort[0] }.permutation(2) do |i,j|
      transition_probs[ [i,j] ] = nil
    end
    e = 0
    while transition_probs.values.include?(nil)
      e += 1
      str.update_gn(g)
      transition_probs.select {|k,v| v.nil? }.each do |k,v|
        if g.is_accessible?( k[0], k[1] )
          transition_probs[k] = e
        end
      end
    end
    pp transition_probs
  end
  calc_transition_probs(str)

  # g(S, WSLS)
  def g_s_wsls(str)
    File.open("g_s_wsls.dot", 'w') do |io|
      bits = 64.times.each.map {|i| (i[0] == i[3]) ? 'c' : 'd' }.join  # i[0],i[3] : the last move of b and a
      wsls = Strategy.make_from_str(bits)
      io.puts str.transition_graph_with(wsls).to_dot
    end
    $stderr.puts "g_s_wsls.dot was written"
  end
  g_s_wsls(str)

  # automaton representation
  uf, min_g = DFAminimize.minimize_DFA(str)
  pp uf.to_h
  puts "automaton size : #{uf.to_h.size}"

  def trace_path(str, init_state = 'cccccd')
    path = []
    s = State.make_from_str(init_state)
    until path.include?(s)
      path.push(s)
      s = str.next_state_with_self(s)
    end
    path
  end

  path = trace_path(str)
  puts "recovered in #{path.length-1} rounds"
  puts path.map {|s| "#{s} (#{uf.root(s.to_id)},#{uf.root(s.swap.to_id)})" }.join(' -> ')

  path = trace_path(str, 'dddddc')
  puts "ends in #{path.length-1} rounds"
  puts path.map {|s| "#{s} (#{uf.root(s.to_id)},#{uf.root(s.swap.to_id)})" }.join(' -> ')

  def to_dot(str, uf, min_g)
    mapped = uf.roots.map do |n|
      [ n, {label: "#{str.action(n)}@#{n}"} ]
    end
    attr = Hash[mapped]
    link_label = Hash.new {|h,k| h[k] = [] }
    t = uf.to_h
    str.transition_graph.for_each_link do |i,j|
      e = [uf.root(i), uf.root(j)]
      h2a = lambda {|x| [8,1].map{|m| ((x&m)==m)?'d':'c'}.join }
      link_label[e].push( h2a.call(j) )
    end
    link_label = link_label.map {|k,v| [k, v.sort.uniq.join(',')] }.to_h
    sio = StringIO.new
    min_g.to_dot(remove_isolated: true, node_attributes: attr, edge_labels: link_label)
  end

  File.open('a.dot', 'w') do |io|
    io.puts to_dot(str, uf, min_g)
  end
  $stderr.puts "a.dot was written"
end
