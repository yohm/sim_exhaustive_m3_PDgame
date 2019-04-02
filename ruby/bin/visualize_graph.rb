require_relative "../lib/strategy"
require_relative "../lib/strategy_m3"

unless ARGV.size == 2
  $stderr.puts "[Error] Usage: ruby #{__FILE__} strategy_string output.pdf"
  raise "invalid argument"
end

if ARGV[0].size == 40
  stra = Strategy.make_from_bits(ARGV[0])
  g = stra.transition_graph_with_self
  node_attributes = Strategy.node_attributes
elsif ARGV[0].size == 512
  stra = StrategyM3.make_from_bits(ARGV[0])
  g = stra.transition_graph_with_self
  node_attributes = StrategyM3.node_attributes
end

tmp = "temp.dot"
io = File.open(tmp, 'w')
g.to_dot( io, node_attributes: node_attributes )
io.close

cmd = "dot -K fdp -T pdf #{tmp} -o #{ARGV[1]}"
system(cmd)

