require 'pp'
require_relative 'state'
require_relative 'graph'

class Strategy

  def initialize( actions )
    raise unless actions.all? {|a| a == :c or a == :d }
    @strategy = actions.dup
    @strategy.freeze
  end

  def to_bits
    @strategy.join('')
  end

  def show_actions(io)
    State::ALL_STATES.each_with_index do |stat,idx|
      io.print "#{@strategy[idx]}|#{stat}\t"
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

  def self.make_from_bits( bits )
    raise "invalid format" unless bits =~ /\A[cd]{64}\z/
    actions = bits.each_char.map(&:to_sym)
    self.new( actions )
  end

  def action( state )
    @strategy[state.to_i]
  end

  def valid?
    @strategy.all? {|a| a == :c or a == :d }
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
    a1 = AMatrix.construct_a1_matrix(self) # construct_a1_matrix
    a = AMatrix.construct_a1_matrix(self)
    return false if a.has_negative_diagonal?

    63.times do |t|
      a.update( a1 )
      return false if a.has_negative_diagonal?
    end
    true
  end

  class AMatrix  # class used for judging defensibility

    N=64

    def self.construct_a1_matrix(stra)
      a = self.new

      N.times do |i|
        s = FullState.make_from_id(i)
        N.times do |j|
          a.a[i][j] = :inf
        end
        ns = stra.possible_next_states(s)
        ns.each do |n|
          j = n.to_id
          a.a[i][j] = n.relative_payoff
        end
      end
      a_b
    end

    attr_reader :a

    def initialize
      @a = Array.new(64) {|i| Array.new(64,0) }
    end

    def inspect
      sio = StringIO.new
      @a.size.times do |i|
        @a[i].size.times do |j|
          if @a[i][j] == :inf
            sio.print(" ##,")
          else
            sio.printf("%3d,", @a[i][j])
          end
        end
        sio.print "\n"
      end
      sio.string
    end

    def has_negative_diagonal?
      @a.size.times do |i|
        if @a[i][i] != :inf and @a[i][i] < 0
          return true
        end
      end
      false
    end

    def update( a1 )
      temp = Array.new(64) {|i| Array.new(64,:inf) }

      @a.size.times do |i|
        @a.size.times do |j|
          @a.size.times do |k|
            x = @a[i][k]
            y = a1.a[k][j]
            next if x == :inf or y == :inf
            temp[i][j] = x+y if temp[i][j] == :inf or x+y < temp[i][j]
          end
        end
      end
      @a = temp
    end
  end

end

if __FILE__ == $0 and ARGV.size != 1
  require 'minitest/autorun'

  class StrategyTest < Minitest::Test

    def test_allD
      bits = "d"*40
      strategy = Strategy.make_from_bits(bits)
      assert_equal bits, strategy.to_bits
      assert_equal :d, strategy.action([:c,:c,0,0] )
      assert_equal :d, strategy.action([:d,:d,2,2] )

      s = FullState.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_full_states(s).map(&:to_s)
      expected = ['cdccdc', 'cdccdd', 'cdcddc', 'cdcddd']
      assert_equal expected, nexts

      next_state = strategy.next_full_state_with_self(s)
      assert_equal 'cdcddd', next_state.to_s

      assert_equal true, strategy.defensible?
    end

    def test_allC
      bits = "c"*40
      strategy = Strategy.make_from_bits(bits)
      assert_equal bits, strategy.to_bits
      assert_equal :c, strategy.action([:c,:c,0,0] )
      assert_equal :c, strategy.action([:d,:d,2,2] )

      s = FullState.new(:c,:c,:d,:c,:c,:d)
      nexts = strategy.possible_next_full_states(s).map(&:to_s)
      expected = ['ccccdc', 'ccccdd', 'cccddc', 'cccddd']
      assert_equal expected, nexts

      next_state = strategy.next_full_state_with_self(s)
      assert_equal 'ccccdc', next_state.to_s

      assert_equal false, strategy.defensible?
    end

    def test_a_strategy
      bits = "ccccdddcdddccccddcdddccccddcddcccccddddd"
      strategy = Strategy.make_from_bits(bits)
      assert_equal bits, strategy.to_bits
      assert_equal :c, strategy.action([:c,:c,0,0] )
      assert_equal :d, strategy.action([:d,:d,2,2] )

      s = FullState.new(:c,:c,:d,:c,:c,:d)
      sid = State.index(s.to_ss)
      move_a = strategy.action([:c,:c,1,1])  #=> d
      nexts = strategy.possible_next_full_states(s).map(&:to_s)
      expected = ['cdccdc', 'cdccdd', 'cdcddc', 'cdcddd']
      assert_equal expected, nexts

      next_state = strategy.next_full_state_with_self(s)
      move_b = strategy.action([:d,:c,0,1])
      move_c = strategy.action([:c,:d,1,0])
      assert_equal "c#{move_a}c#{move_b}d#{move_c}", next_state.to_s

      assert_equal false, strategy.defensible?
    end

    def test_most_generous_PS2
      bits = "cddcdddcddcccdcddddddcccdddcccccddcddddd"
      strategy = Strategy.make_from_bits(bits)
      assert_equal bits, strategy.to_bits
      assert_equal :c, strategy.action([:c,:c,0,0] )
      assert_equal :d, strategy.action([:d,:d,2,2] )

      s = FullState.new(:c,:c,:d,:c,:c,:d)
      move_a = strategy.action([:c,:c,1,1]) #=> d
      nexts = strategy.possible_next_full_states(s).map(&:to_s)
      expected = ['cdccdc', 'cdccdd', 'cdcddc', 'cdcddd']
      assert_equal expected, nexts

      next_state = strategy.next_full_state_with_self(s)
      move_b = strategy.action([:d,:c,0,1])
      move_c = strategy.action([:c,:d,1,0])
      assert_equal "c#{move_a}c#{move_b}d#{move_c}", next_state.to_s
      assert_equal true, strategy.defensible?
    end

  end
end

if __FILE__ == $0 and ARGV.size == 1
  bits = ARGV[0]
  stra = Strategy.make_from_bits(bits)
  stra.show_actions($stdout)
  stra.show_actions_using_full_state($stdout)
  #stra.transition_graph_with_self.to_dot($stdout)
  #stra.show_actions_latex($stdout)
  #a1_b, a1_c = Strategy::AMatrix.construct_a1_matrix(stra)
  #pp a1_b
  #pp a1_b.has_negative_diagonal?
  #a1_b.update(a1_b)
  #pp a1_b
  #pp "def: #{stra.defensible?}"
end

