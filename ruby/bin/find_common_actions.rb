require 'pp'
require_relative '../lib/strategy'

unless ARGV.size == 1
  $stderr.puts "[usage] ruby #{__FILE__} strategies.txt"
  raise "invalid argument"
end


strategies = File.open(ARGV[0]).map do |line|
  line.chomp
end

pattern = strategies[0].dup

strategies.each_with_index do |line,idx|
  $stderr.puts idx if idx % 100000 == 0
  line.size.times do |i|
    if pattern[i] and pattern[i] != line[i]
      pattern[i] = '-'
    end
  end
end

puts pattern

indexes = pattern.each_char.map.with_index {|c,i| i if c == '-' }.compact
indexes.each do |i|
  actions = strategies.map {|chars| chars[i] }
  histo = Hash.new(0)
  actions.each {|act| histo[act] += 1 }
  # p i, histo
  if histo.values.all? {|x| x == strategies.size/2 }
    pattern[i] = '*'
  end
end

puts pattern

indexes = pattern.each_char.map.with_index {|c,i| i if c=='-' }.compact
shortened = strategies.map do |s|
  indexes.map {|i| s[i] }.join
end

puts shortened.uniq

