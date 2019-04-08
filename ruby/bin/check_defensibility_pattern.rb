require 'pp'
require 'pry'
require_relative '../lib/graph'
require_relative '../lib/state'
require_relative '../lib/meta_strategy'

$b_moves = nil

def explore(stra, state, t, &block)
  # pp state
  if t == $b_moves.size
    block.call(stra)
    return
  end
  a_move = stra.action(state)
  # pp "a_move: #{a_move}"
  b_move = $b_moves[t]
  if a_move != :_
    ns = state.next_state(a_move, b_move)
    explore(stra, ns, t+1, &block)
  else
    [:c,:d].each do |a_move|
      s = stra.dup
      s.set(state, a_move)
      ns = state.next_state(a_move, b_move)
      explore(s, ns, t+1, &block)
    end
  end
end

def select_defensible( strategies, init_state, b_moves )
  $b_moves = b_moves
  found = []
  strategies.each do |stra|
    explore(stra, init_state, 0) do |s|
      found << s if s.defensible?
    end
  end
  found
end

# strategies = $stdin.readlines.each.map do |l|
#   MetaStrategy.make_from_str(l.chomp)
# end

strategies = %w(
_______d_______c_______d_______d_______c_______d_______c_______d
).map {|s| MetaStrategy.make_from_str(s) }

# B-states : ddd -> ddc -> dcd -> ddc -> ddd
found = select_defensible( strategies, State.make_from_str('dddddd'), %i(c d d d) )
puts found

