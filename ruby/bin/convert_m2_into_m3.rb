require_relative '../lib/strategy_m3'

if ARGV.size == 1 and ARGV[0].length == 40
  stra = Strategy.make_from_bits(ARGV[0])
  m3_stra = StrategyM3.make_from_m2_strategy(stra)
  m3_stra.show_actions($stdout)
  puts m3_stra.to_bits
elsif ARGV.size == 1
  File.open(ARGV[0]).each do |line|
    stra = Strategy.make_from_bits(line.chomp)
    m3_stra = StrategyM3.make_from_m2_strategy(stra)
    m3_stra.make_successful
    puts m3_stra.to_bits
  end
else
  $stdout.puts "[Error] Usage: ruby #{__FILE__} strategies.txt"
  $stdout.puts "           or: ruby #{__FILE__} strategy"
  raise "invalid argument"
end

