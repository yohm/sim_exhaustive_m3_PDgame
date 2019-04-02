require_relative "../lib/strategy"
require_relative "../lib/strategy_m3"

unless ARGV.size == 1
  $stderr.puts "[Error] Usage: ruby #{__FILE__} strategies.txt"
  raise "invalid argument"
end

def connected_components(g, dest)
  comp = []
  g.n.times do |ni|
    if g.is_accessible?(ni, dest)
      comp << ni
    end
  end
  comp
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
  c_size = connected_components(g, 0).size # reachable to ccc
  d_size = connected_components(g, g.n-1).size # reachable to ddd
  puts "#{s} #{c_size} #{d_size}"
end

