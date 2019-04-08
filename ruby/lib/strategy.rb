require 'pp'
require_relative 'state'
require_relative 'graph'
require 'stringio'

class Strategy

  def initialize( actions )
    raise unless actions.size == 64
    raise unless actions.all? {|a| a == :c or a == :d }
    @strategy = actions.dup
  end

  def to_s
    @strategy.join('')
  end

  def inspect
    sio = StringIO.new
    sio.puts to_s
    State::ALL_STATES.each_with_index do |stat,idx|
      sio.print "#{@strategy[idx]}|#{stat.map(&:to_s).join}\t"
      sio.print "\n" if idx % 8 == 7
    end
    State.c(sio.string)
  end

  def show_actions(io)
    State::ALL_STATES.each_with_index do |stat,idx|
      io.print "#{@strategy[idx]}|#{stat.map(&:to_s).join}\t"
      io.print "\n" if idx % 8 == 7
    end
  end

  def show_actions_latex(io)
    raise "not implemented yet"
=begin
    num_col = 4
    num_row = State::ALL_STATES.size / num_col
    num_row.times do |row|
      num_col.times do |col|
        idx = row + col * num_row
        stat = State::ALL_STATES[idx]
        s = stat.map do |c|
          if c == -1
            '\bar{1}'
          elsif c.is_a?(Integer)
            c.to_s
          else
            c.capitalize
          end
        end
        s.insert(2,',')
        io.print "$(#{s.join})$ & $#{@strategy[stat].capitalize}$ "
        io.print "& " unless col == num_col - 1
      end
      io.puts "\\\\"
    end
=end
  end

  def dup
    self.class.new( @strategy )
  end

  def self.make_from_str( bits )
    raise "invalid format" unless bits =~ /\A[cd]{64}\z/
    actions = bits.each_char.map(&:to_sym)
    self.new( actions )
  end

  def action( state )
    if state.is_a? State
      @strategy[state.to_i]
    elsif state.is_a? Array
      s = State.new(*state)
      @strategy[s.to_i]
    elsif state.is_a? String
      s = State.make_from_str(state)
      @strategy[s.to_i]
    else
      raise "invalid input"
    end
  end

  def valid?
    @strategy.all? {|a| a == :c or a == :d }
  end

  def set( state, act )
    raise unless act == :c or act == :d
    if state.is_a? State
      @strategy[state.to_i] = act
    elsif state.is_a? Array
      s = State.new(*state)
      @strategy[s.to_i] = act
    elsif state.is_a? String
      s = State.make_from_str(state)
      @strategy[s.to_i] = act
    else
      raise "invalid input"
    end
  end

  def possible_next_states(current)
    act_a = action(current)
    n1 = current.next_state(act_a,:c)
    n2 = current.next_state(act_a,:d)
    [n1,n2]
  end

  def next_state_with_self(current)
    act_a = action(current)
    act_b = action(current.swap)
    current.next_state(act_a,act_b)
  end

  def transition_graph
    g = DirectedGraph.new(64)
    64.times do |i|
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
      s = State.make_from_id(i)
      n = next_state_with_self(s)
      g.add_link( i, n.to_i )
    end
    g
  end

  def self.node_attributes
    node_attributes = {}
    64.times do |i|
      s = State.make_from_id(i)
      node_attributes[i] = {}
      node_attributes[i][:label] = "#{i}_#{s}"
    end
    node_attributes
  end

  def defensible?
    g = weighted_transition_graph
    !(g.has_negative_cycle?)
  end

  def weighted_transition_graph
    g = DirectedWeightedGraph.new(64)
    64.times do |i|
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

    def test_allD
      bits = "d"*64
      strategy = Strategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:d,:d,:d,:d,:d])
      assert_equal :d, strategy.action([:d,:d,:c,:d,:d,:c])

      s = State.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cddcdc', 'cddcdd']
      assert_equal expected, nexts

      next_state = strategy.next_state_with_self(s)
      assert_equal 'cddcdd', next_state.to_s
      assert_equal true, strategy.defensible?
    end

    def test_allC
      bits = "c"*64
      strategy = Strategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :c, strategy.action([:c,:c,:c,:c,:c,:c])
      assert_equal :c, strategy.action([:d,:d,:d,:d,:d,:d])

      s = State.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdccdc','cdccdd']
      assert_equal expected, nexts

      next_state = strategy.next_state_with_self(s)
      assert_equal 'cdccdc', next_state.to_s

      assert_equal false, strategy.defensible?
    end

    def test_tft
      bits = "cd"*32
      strategy = Strategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :c, strategy.action([:d,:c,:d,:c,:d,:c] )
      assert_equal :d, strategy.action([:d,:c,:d,:c,:d,:d] )

      s = State.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cddcdc', 'cddcdd']
      assert_equal expected, nexts

      next_state = strategy.next_state_with_self(s)
      assert_equal 'cddcdd', next_state.to_s

      assert_equal true, strategy.defensible?
    end

    def test_wsls
      bits = 64.times.each.map {|i| (i[0] == i[3]) ? 'c' : 'd' }.join  # i[0],i[3] : the last move of b and a
      strategy = Strategy.make_from_str(bits)
      assert_equal bits, strategy.to_s
      assert_equal :d, strategy.action([:d,:c,:d,:c,:d,:c] )
      assert_equal :c, strategy.action([:d,:c,:d,:c,:d,:d] )

      s = State.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_states(s).map(&:to_s)
      expected = ['cdccdc', 'cdccdd']
      assert_equal expected, nexts

      next_state = strategy.next_state_with_self(s)
      assert_equal 'cdccdc', next_state.to_s

      assert_equal false, strategy.defensible?
    end
  end
end

if __FILE__ == $0 and ARGV.size == 1
  pp Strategy.make_from_str(ARGV[0])
end

