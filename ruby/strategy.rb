require 'pp'
require_relative 'state'
require_relative 'graph'
require 'stringio'

class Strategy

  A = [:c,:d]

  def initialize( actions )
    raise unless actions.size == 64
    raise unless actions.all? {|a| self.class::A.include?(a) }
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

  def show_actions_latex
    io = StringIO.new
    num_col = 8
    num_row = 8
    # header
    b_stats = num_col.times.each.map do |col|
      "$#{State.make_from_id(col).to_s[3..5]}$"
    end
    io.puts b_stats.join(" & ") + "\\\\"
    num_row.times do |row|
      l = num_col.times.each.map do |col|
        idx = row * num_col + col
        "$#{action(idx)}$"
      end.join(" & ")
      io.puts "#{l} \\\\"
    end
    io.string
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
    s =
      case state
      when State
        state
      when Array
        State.new(*state)
      when String
        State.make_from_str(state)
      when Integer
        State.make_from_id(state)
      else
        raise "invalid input"
      end
    @strategy[s.to_i]
  end

  def valid?
    @strategy.all? {|a| self.class::A.include?(a) }
  end

  def set( state, act )
    raise "#{self.class::A.inspect}" unless self.class::A.include?(act)
    s =
      case state
      when State
        state
      when Array
        State.new(*state)
      when String
        State.make_from_str(state)
      else
        raise "invalid input"
      end
    @strategy[s.to_i] = act
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

  def transition_graph_with( other_s )
    g = DirectedGraph.new(64)
    64.times do |i|
      s = State.make_from_id(i)
      n = s.next_state( action(s), other_s.action(s.swap) )
      g.add_link(i, n.to_i)
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

  def efficient?
    g0 = transition_graph_with_self

    judged = Array.new(64, false)
    judged[0] = true

    g = g0

    while true
      # l -> 0
      judged.each_with_index do |b,l|
        next if b
        judged[l] = true if g.is_accessible?(l, 0)
      end
      return true if judged.all?

      # update gn
      update_gn(g)

      # 0 -> l
      judged.each_with_index do |b,l|
        next if b
        return false if g.is_accessible?(0, l)
      end
    end
  end

  def distinguishable?
    allc = Strategy.make_from_str('c'*64)
    g = transition_graph_with(allc)

    judged = Array.new(64, false)
    judged[0] = true

    while true
      # l -> 0
      judged.each_with_index do |b,l|
        next if b
        judged[l] = true if g.is_accessible?(l, 0)
      end
      return false if judged.all?

      # update gn
      update_gn(g)

      # 0 -> l
      judged.each_with_index do |b,l|
        next if b
        return true if g.is_accessible?(0, l)
      end
    end
  end

  def update_gn(gn)
    # find sink sccs
    sink_sccs = gn.terminanl_components.select do |c|
      c.all? do |n|
        gn.links[n].all? do |d|
          c.include?(d)
        end
      end
    end

    sink_sccs.each do |sink|
      sink.each do |from|
        [from^1,from^8].each do |to|
          unless gn.links[from].include?(to)
            gn.add_link(from, to)
          end
        end
      end
    end
    return gn
  end
end

if __FILE__ == $0 and ARGV.size == 0
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
      assert_equal false, strategy.efficient?
      assert_equal true, strategy.distinguishable?
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
      assert_equal true, strategy.efficient?
      assert_equal false, strategy.distinguishable?
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
      assert_equal false, strategy.efficient?
      assert_equal false, strategy.distinguishable?
    end

    def test_tft_atft
      m2_actions = "cdcddccdcdccdccd".each_char.map(&:to_sym)
      acts = 64.times.each.map do |i|
        m2_idx = (i & 3) + ((i & 24) >> 1)
        m2_actions[m2_idx]
      end
      strategy = Strategy.new(acts)
      assert_equal strategy.to_s, "cdcdcdcddccddccdcdcccdccdccddccdcdcdcdcddccddccdcdcccdccdccddccd"

      assert_equal true, strategy.defensible?
      assert_equal true, strategy.efficient?
      assert_equal true, strategy.distinguishable?

      recovery_path = [1]
      until recovery_path.last == 0
        i = strategy.next_state_with_self( State.make_from_id(recovery_path.last) ).to_id
        recovery_path << i
      end
      # ccc,ccd -> ccd,cdd -> cdd,ddc -> ddc,dcc -> dcc,ccc -> ccc,ccc
      assert_equal [1,11,30,52,32,0], recovery_path
    end

    def test_tft_atft_variants
      m2_tft_atfts = [
        "cdcddccdcdccdccd",
        "cdcdddcdcdccdccd",
        "cdcddccdcddcdccd",
        "cdcdddcdcddcdccd"
      ].map {|s| s.each_char.map(&:to_sym) }
      m2_tft_atfts.each do |m2_actions|
        acts = 64.times.each.map do |i|
          m2_idx = (i & 3) + ((i & 24) >> 1)
          m2_actions[m2_idx]
        end
        strategy = Strategy.new(acts)
        assert_equal true, strategy.defensible?
        assert_equal true, strategy.efficient?
        assert_equal true, strategy.distinguishable?
      end
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
      assert_equal true, strategy.efficient?
      assert_equal true, strategy.distinguishable?
    end
  end
elsif __FILE__ == $0 and ARGV.size == 1
  pp s = Strategy.make_from_str(ARGV[0])
  $stderr.puts s.show_actions_latex
  g = s.transition_graph_with_self
  $stderr.puts "  defensible? : #{s.defensible?}"
  $stderr.puts "  efficient?  : #{s.efficient?}"
  $stderr.puts "  distinguish?: #{s.distinguishable?}"

  # analyze g(S,S)
  File.open("g_ss.dot", 'w') do |io|
    io.puts g.to_dot
  end
  $stderr.puts "g_ss.dot was written"
  sccs = g.terminanl_components
  $stderr.puts "SCC in g(S,S) : #{sccs.inspect}"
  transition_probs = {}
  sccs.map {|c| c[0] }.permutation(2) do |i,j|
    transition_probs[ [i,j] ] = nil
  end
  e = 0
  while transition_probs.values.include?(nil)
    e += 1
    s.update_gn(g)
    transition_probs.select {|k,v| v.nil? }.each do |k,v|
      if g.is_accessible?( k[0], k[1] )
        transition_probs[k] = e
      end
    end
  end
  $stderr.puts "transition probs between SCCs: #{transition_probs.inspect}"
  
  # g(S, AllC)
  def g_s_allc(str)
    File.open("g_s_allc.dot", 'w') do |io|
      bits = 'c' * 64
      allc = Strategy.make_from_str(bits)
      io.puts str.transition_graph_with(allc).to_dot
    end
    $stderr.puts "g_s_allc.dot was written"
  end
  g_s_allc(s)

  # g(S, WSLS)
  def g_s_wsls(str)
    File.open("g_s_wsls.dot", 'w') do |io|
      bits = 64.times.each.map {|i| (i[0] == i[3]) ? 'c' : 'd' }.join  # i[0],i[3] : the last move of b and a
      wsls = Strategy.make_from_str(bits)
      io.puts str.transition_graph_with(wsls).to_dot
    end
    $stderr.puts "g_s_wsls.dot was written"
  end
  g_s_wsls(s)
elsif __FILE__ == $0 and ARGV.size == 2
  str1 = Strategy.make_from_str(ARGV[0])
  str2 = Strategy.make_from_str(ARGV[1])

  File.open("g_s1_s2.dot", 'w') do |io|
    io.puts str1.transition_graph_with(str2).to_dot
    $stderr.puts "g_s1_s2.dot was written"
  end
end

