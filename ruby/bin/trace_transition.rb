require_relative "../lib/strategy"
require_relative "../lib/strategy_m3"

unless ARGV.size >= 2
  $stderr.puts "[Error] Usage: ruby #{__FILE__} strategies.txt start_state_id [ids...]"
  raise "invalid argument"
end

def last_node( g, start )
  c = start
  n = g.links[c].first
  g.n.times do |t|
    return n if c == n
    c = n
    n = g.links[c].first
  end
  nil   # no termination node is found
end

File.open(ARGV[0]).each do |line|
  s = line.chomp
  if s.size == 40
    stra = Strategy.make_from_bits(s)
    g = stra.transition_graph_with_self
  elsif s.size == 512
    stra = StrategyM3.make_from_bits(s)
    g = stra.transition_graph_with_self
  else
    raise "must not happen"
  end
  last_nodes = ARGV[1..-1].map do |arg|
    last_node(g, arg.to_i)
  end
  puts last_nodes.join(' ')
end

