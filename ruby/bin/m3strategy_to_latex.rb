require_relative '../lib/strategy_m3'

unless ARGV.size == 1
  $stderr.puts "Usage: ruby #{__FILE__} <m3_strategy>"
  raise "invalid argument"
end

stra = StrategyM3.make_from_bits(ARGV[0])
stra.show_actions_latex($stdout)

