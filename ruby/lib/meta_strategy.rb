require 'pp'
require_relative 'state'
require_relative 'graph'
require_relative 'strategy'

class MetaStrategy < Strategy

  def initialize( actions )
    raise unless actions.size == 64
    raise unless actions.all? {|a| a == :c or a == :d or a == :_ }  # :_ denotes a wildcard
    @strategy = actions.dup
    @strategy.freeze
  end

  def self.make_from_str( bits )
    raise "invalid format" unless bits =~ /\A[cd_]{64}\z/
    actions = bits.each_char.map(&:to_sym)
    self.new( actions )
  end

  def action( state )
    if state.is_a? State
      @strategy[state.to_i]
    elsif state.is_a? Array
      s = State.new(*state)
      @strategy[s.to_i]
    else
      raise "invalid input"
    end
  end

  def valid?
    @strategy.all? {|a| a == :c or a == :d or a == :_ }
  end

  def possible_next_states(current)
    act_a = action(current)
    return nil if act_a == :_
    n1 = current.next_state(act_a,:c)
    n2 = current.next_state(act_a,:d)
    [n1,n2]
  end

  def next_state_with_self(current)
    act_a = action(current)
    act_b = action(current.swap)
    return nil if act_a == :_ or act_b == :_
    current.next_state(act_a,act_b)
  end

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

  def transition_graph_with_self
    g = DirectedGraph.new(64)
    64.times do |i|
      next if @strategy[i] == :_
      s = State.make_from_id(i)
      n = next_state_with_self(s)
      g.add_link( i, n.to_i )
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
        g.add_link(i,j,n.relative_payoff)
      end
    end
    g
  end
end

if __FILE__ == $0 and ARGV.size != 1
  require 'minitest/autorun'

  class StrategyTest < Minitest::Test

    def test_allD_against_allD
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          'd'
        else
          '_'
        end
      }.join
      strategy = MetaStrategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cddddc', 'cddddd']
      assert_equal expected, nexts

      assert_nil strategy.next_state_with_self(s)

      s = State.new(:d,:d,:d,:d,:d,:d)
      n = strategy.next_state_with_self(s)
      assert_equal 'dddddd', n.to_s

      assert_equal true, strategy.defensible?
    end

    def test_allC_against_allD
      bits = 64.times.each.map {|i|
        if i[0]==1 and i[1]==1 and i[2]==1  # fix action only for the states (***ddd)
          'c'
        else
          '_'
        end
      }.join
      strategy = MetaStrategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :c, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdcddc', 'cdcddd']
      assert_equal expected, nexts

      assert_nil strategy.next_state_with_self(s)

      s = State.new(:d,:d,:d,:d,:d,:d)
      n = strategy.next_state_with_self(s)
      assert_equal 'ddcddc', n.to_s

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
      strategy = MetaStrategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:d])
      assert_equal :_, strategy.action([:d,:d,:d,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      assert_nil strategy.possible_next_states(s) # returns nil when the next state is not determined

      s = State.new(:c,:c,:d,:d,:d,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdcddc', 'cdcddd']
      assert_equal expected, nexts

      assert_nil strategy.next_state_with_self(s)

      s = State.new(:d,:d,:d,:d,:d,:d)
      n = strategy.next_state_with_self(s)
      assert_equal 'ddcddc', n.to_s

      assert_equal false, strategy.defensible?
    end

  end
end

if __FILE__ == $0 and ARGV.size == 1
  bits = ARGV[0]
  stra = Strategy.make_from_str(bits)
  stra.show_actions($stdout)
end

