require 'pp'
require_relative '../lib/strategy'
require_relative '../lib/strategy_m3'

unless ARGV.size == 1
  $stderr.puts "[usage] ruby #{__FILE__} strategies.txt"
  raise "invalid argument"
end


graphs = File.open(ARGV[0]).map do |line|
  if line.chomp.size == 40
    str = Strategy.make_from_bits(line.chomp)
    str.transition_graph_with_self
  elsif line.chomp.size == 512
    str = StrategyM3.make_from_bits(line.chomp)
    str.transition_graph_with_self
  else
    raise "unknown format"
  end
end

g = graphs.inject {|memo,g| DirectedGraph.common_subgraph(memo,g) }

if g.n == 64
  node_attributes = Strategy.node_attributes
else
  node_attributes = StrategyM3.node_attributes
end
g.to_dot($stdout, remove_isolated: true, node_attributes: node_attributes)

node_sets = graphs.map do |g|
  nodes = []
  g.n.times do |i|
    nodes.push(i) if g.is_accessible?(i,0)
  end
  nodes
end
common_nodes = node_sets.inject {|memo,nodes| memo & nodes }
p common_nodes, common_nodes.size

node_sets = graphs.map do |g|
  nodes = []
  g.n.times do |i|
    nodes.push(i) if g.is_accessible?(i,g.n-1)
  end
  nodes
end
common_nodes = node_sets.inject {|memo,nodes| memo & nodes }
p common_nodes, common_nodes.size

