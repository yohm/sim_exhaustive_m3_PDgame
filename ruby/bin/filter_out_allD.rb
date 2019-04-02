require 'pp'
require_relative '../lib/graph'
require_relative '../lib/state'

module DefensibleAllD

  # Since either B or C is allD, the states cannot be [*,0] or [0,*], or [1,1].
  BC_STATES = [
      #[0,0],
      #[0,1],
      #[0,2],
      #[1,0],
      #[1,1],
      [1,-1],
      [1,2],
      #[2,0],
      [2,1],
      [2,2]
  ]

  SUB_STATES = []
  State::A_STATES.each do |a|
    BC_STATES.each do |b|
      SUB_STATES << a+b
    end
  end
  #$stderr.puts "SUB_STATES : ", SUB_STATES.inspect

  FIXED_STRATEGY = {
      [:d,:d,2, 2] => :d,
      [:d,:d,2, 1] => :d,
      [:d,:d,1, 2] => :d,
      [:d,:d,1,-1] => :d,
      [:c,:d,2, 2] => :d,
      [:c,:d,1,-1] => :d,
      [:c,:c,2, 2] => :d,
      [:c,:c,1,-1] => :d,
      #[:c,:c,0, 0] => :c,
      [:c,:c,1, 1] => :d  # defensibility against (11)
      #[:c,:d,1, 2] => :d,
      #[:c,:d,2, 1] => :d
  }

  UNFIXED_STATES = SUB_STATES - FIXED_STRATEGY.keys
  # pp "UNFIXED_STATES: ", UNFIXED_STATES

  def self.strategy_candidates
    strategy_candidates = [FIXED_STRATEGY]
    UNFIXED_STATES.each do |state|
      copy1 = Marshal.load( Marshal.dump( strategy_candidates ) )
      copy1.each {|strategy| strategy[state] = :c }
      copy2 = Marshal.load( Marshal.dump( strategy_candidates ) )
      copy2.each {|strategy| strategy[state] = :d }
      strategy_candidates = copy1 + copy2
    end
    #$stderr.puts strategy_candidates.size
    strategy_candidates
  end

  def self.possible_next_states( strategy, stat )
    next_stat = [nil, nil, nil, nil]
    next_stat[0] = stat[1]
    next_stat[2] = (stat[3]!=-1) ? stat[3] : 1

    next_stat[1] = strategy[stat]

    if next_stat[2] == 1
      possible_states = [-1,2].map do |act_bc|
        next_stat[3] = act_bc
        next_stat.dup
      end
    else
      possible_states = [1,2].map do |act_bc|
        next_stat[3] = act_bc
        next_stat.dup
      end
    end
    possible_states
  end

  def self.construct_transition_graph(strategy)
    g = DirectedGraph.new(SUB_STATES.size )
    SUB_STATES.each_with_index do |stat,idx|
      next_states = possible_next_states( strategy, stat )
      #pp "stat", stat, "next_stat", next_states
      next_indexes = next_states.map {|next_stat| SUB_STATES.index(next_stat) }
      #pp next_indexes
      next_indexes.each do |n|
        g.add_link(idx, n)
      end
    end
    g
  end

  def self.defensible_strategies
    strategies = []
    strategy_candidates.each do |str|
      g = construct_transition_graph(str)
      risky_state = g.non_transient_nodes.find do |n|
        s = SUB_STATES[n]
        #!(s[0] == :d and s[1] == :d)
        s[1] == :c
      end
      #pp "risky_state : #{risky_state}"
      is_defensible = risky_state.nil?
      strategies << str if is_defensible
    end
    strategies
  end
end

def strategy_to_bits( strategy )
  State::ALL_STATES.map do |stat|
    (strategy[stat] || '-').to_s
  end.join
end

defensible_strategies = DefensibleAllD.defensible_strategies
defensible_strategies.each do |str|
  puts strategy_to_bits(str)
end

