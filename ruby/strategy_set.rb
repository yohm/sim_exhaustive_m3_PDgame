require 'pp'
require_relative 'state'
require_relative 'graph'
require_relative 'strategy'

class StrategySet < Strategy

  # _: undefined action, *: wild-card
  A = [:c,:d,:_,:*]

  def self.make_from_str( bits )
    raise "invalid format" unless bits =~ /\A[cd_*]{64}\z/
    actions = bits.each_char.map(&:to_sym)
    self.new( actions )
  end

  def possible_next_states(current)
    act_a = action(current)
    return nil if act_a == :_
    if act_a == :*
      [current.next_state(:c,:c),current.next_state(:c,:d),
       current.next_state(:d,:c),current.next_state(:d,:d)]
    else
      [current.next_state(act_a,:c), current.next_state(act_a,:d)]
    end
  end

  # returns g(S,*)
  def transition_graph
    g = DirectedGraph.new(64)
    64.times do |i|
      next if @strategy[i] == :_
      s = State.make_from_id(i)
      next_ss = possible_next_states(s)
      next_ss.each do |n|
        g.add_link(i,n.to_i)
      end
    end
    g
  end

  def next_states_with_self(current, skip_unfixed=false)
    act_a = action(current)
    if act_a == :_ or act_a == :*
      acts_a = (skip_unfixed) ? [] : [:c,:d]
    else
      acts_a = [act_a]
    end
    act_b = action(current.swap)
    if act_b == :_ or act_b == :*
      acts_b = (skip_unfixed) ? [] : [:c,:d]
    else
      acts_b = [act_b]
    end
    ans = []
    acts_a.each do |a|
      acts_b.each do |b|
        ans << current.next_state(a,b)
      end
    end
    ans
  end

  # returns g(S,S)
  def transition_graph_with_self(skip_unfixed=false)
    g = DirectedGraph.new(64)
    64.times do |i|
      s = State.make_from_id(i)
      ns = next_states_with_self(s, skip_unfixed)
      ns.each do |n|
        g.add_link( i, n.to_i )
      end
    end
    g
  end

  def defensible?
    g = weighted_transition_graph
    !(g.has_negative_cycle?)
  end

  def weighted_transition_graph
    g = DirectedWeightedGraph.new(64)
    64.times do |i|
      next if @strategy[i] == :_
      s = State.make_from_id(i)
      ns = possible_next_states(s)
      ns.each do |n|
        j = n.to_id
        g.add_link(i,j,s.relative_payoff)
      end
    end
    g
  end
end

if __FILE__ == $0 and ARGV.size != 1
  require 'minitest/autorun'

  class StrategySetTest < Minitest::Test

    def test_allD_against_allD
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          'd'
        else
          '_'
        end
      }.join
      strategy = StrategySet.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cddddc', 'cddddd']
      assert_equal expected, nexts

      assert_equal true, strategy.defensible?
    end

    def test_wildcar
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          'd'
        else
          '_'
        end
      }.join
      strategy = StrategySet.make_from_str(bits)
      strategy.set('ddcddc', :*)
      strategy.set('ddcddd', :*)
      strategy.set('cddcdd', :d)
      strategy.set('cddddd', :d)
      strategy.set('dddcdd', :d)
      strategy.set('dddddd', :d)  # `ddcdd*` is unreachable state
      assert_equal true, strategy.defensible?

      s = StrategySet.make_from_str('_'*63+'*')
      s.set('ddcddd',:d)
      s.set('dcdddd',:d)
      s.set('cddddd',:d)
      assert_equal false, s.defensible?
    end

    def test_allC_against_allD
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          'c'
        else
          '_'
        end
      }.join
      strategy = StrategySet.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :c, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdcddc', 'cdcddd']
      assert_equal expected, nexts

      assert_equal false, strategy.defensible?
    end

    def test_wsls_against_allD
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          (i[3] == 1) ? 'c' : 'd'
        else
          '_'
        end
      }.join
      strategy = StrategySet.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdcddc', 'cdcddd']
      assert_equal expected, nexts

      assert_equal false, strategy.defensible?
    end

    def test_tft_against_for_3d2d
      bits = 64.times.each.map {|i|
        if i[0]+i[1]+i[2] >= 2  # (***ddd),(***cdd),(***dcd),(***ddc)
          (i[0] == 1) ? 'd' : 'c'
        else
          '_'
        end
      }.join
      strategy = StrategySet.make_from_str(bits)
      # require 'pry'; binding.pry
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :c, strategy.action([:d,:d,:d,:d,:d,:c])
      assert_equal :_, strategy.action([:d,:c,:d,:c,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:c)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdcdcc', 'cdcdcd']
      assert_equal expected, nexts

      assert_equal true, strategy.defensible?
    end

  end
elsif __FILE__ == $0 and ARGV.size == 1
  s = StrategySet.make_from_str(ARGV[0])
  $stderr.puts s.inspect
  $stderr.puts "defensible? : #{s.defensible?}"
  g = s.weighted_transition_graph
  # g = s.transition_graph_with_self(true)
  $stderr.puts g.sccs.inspect
  scc = g.sccs.find {|x| x.include?(0) }
  distance = g.bellman_ford(0)
  $stderr.puts Hash[scc.map {|i| [i,distance[i]] }.sort_by{|k,v| k}].inspect
  attributes = {}
  64.times.each.map do |i|
    c = scc.include?(i) ? "lawngreen" : "white"
    l = "#{i}_#{sprintf('%06b',i).gsub('0','c').gsub('1','d')}_#{distance[i]}"
    attributes[i] = {label: l, fillcolor: c, style: "filled"}
  end
  edge_attributes = {}
  s.transition_graph_with_self.for_each_link {|i,j|
    edge_attributes[ [i,j] ] = {color: "red"}
  }
  $stdout.puts g.to_dot(node_attributes: attributes, edge_attributes: edge_attributes )
end

