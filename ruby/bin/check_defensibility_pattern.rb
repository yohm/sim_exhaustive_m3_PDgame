require 'pp'
require 'pry'
require_relative '../lib/graph'
require_relative '../lib/state'
require_relative '../lib/meta_strategy'

def explore(stra, state, b_moves, &block)
  # pp state
  if b_moves.empty?
    block.call(stra)
    return
  end
  a_move = stra.action(state)
  _b_moves = b_moves.dup
  b_move = _b_moves.shift
  if a_move != :_
    ns = state.next_state(a_move, b_move)
    explore(stra, ns, _b_moves, &block)
  else
    [:c,:d].each do |a_move|
      s = stra.dup
      s.set(state, a_move)
      ns = state.next_state(a_move, b_move)
      explore(s, ns, _b_moves, &block)
    end
  end
end

def select_defensible( strategies, init_state, b_moves )
  _b_moves = b_moves.dup
  found = []
  strategies.each_with_index do |stra,idx|
    $stderr.puts " #{idx} / #{strategies.size}" if idx % 10 == 0
    explore(stra, init_state, _b_moves) do |s|
      found << s if s.defensible?
    end
  end
  found
end

# strategies = $stdin.readlines.each.map do |l|
#   MetaStrategy.make_from_str(l.chomp)
# end

if ARGV.size == 3
  # usage: ruby #{__FILE__} [strategy file] [initial state] [b moves]
  #    eg: ruby #{__FILE__} strategies.txt dddddd cddd
  lines = File.open(ARGV[0]).readlines
  strategies = lines.map {|l| MetaStrategy.make_from_str(l.chomp) }
  init_state = State.make_from_str(ARGV[1])
  b_moves = ARGV[2].each_char.map(&:to_sym)
  puts select_defensible(strategies, init_state, b_moves)
else
  strategies = %w(
  _______d_______c_______d_______d_______c_______d_______c_______d
  _______d_______c_______d_______d_______c_______d_______d_______d
  _______d_______c_______d_______d_______d_______d_______c_______d
  _______d_______c_______d_______d_______d_______d_______d_______d
  _______d_______d_______c_______d_______c_______c_______c_______d
  _______d_______d_______c_______d_______c_______c_______d_______d
  _______d_______d_______c_______d_______c_______d_______c_______d
  _______d_______d_______c_______d_______c_______d_______d_______d
  _______d_______d_______c_______d_______d_______c_______c_______d
  _______d_______d_______c_______d_______d_______c_______d_______d
  _______d_______d_______c_______d_______d_______d_______c_______d
  _______d_______d_______c_______d_______d_______d_______d_______d
  _______d_______d_______d_______d_______c_______d_______c_______d
  _______d_______d_______d_______d_______c_______d_______d_______d
  _______d_______d_______d_______d_______d_______d_______c_______d
  _______d_______d_______d_______d_______d_______d_______d_______d
  ).map {|s| MetaStrategy.make_from_str(s) }

  # B-states : ddd -> ddc -> dcd -> cdd -> ddd
  found = select_defensible( strategies, State.make_from_str('dddddd'), %i(c d d d) )
  puts found

  # B-states : ddd -> ddc -> dcd -> cdc -> dcd -> cdd -> ddd
  found2 = select_defensible( found, State.make_from_str('dddddd'), %i(c d c d d d) )
  puts found2
end
