require 'pp'

counts = {}
total_counts = {}

File.open(ARGV[0]).each do |line|
  key,c0,c1 = line.chomp.split(':')
  c0 = c0.to_i
  c1 = c1.to_i
  counts[key] = c0
  total_counts[key] = c1
end

# pp counts, total_counts
counts.sort_by {|k,v|v }.to_a[-7..-1].each do |k,v|
  puts "#{k}, #{v}"
end
puts "---"
total_counts.sort_by {|k,v|v }.to_a[-7..-1].each do |k,v|
  puts "#{k}, #{v}"
end

