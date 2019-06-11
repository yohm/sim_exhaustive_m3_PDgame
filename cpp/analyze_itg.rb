require 'pp'

unless ARGV.size == 1
  $stderr.puts "[error] invalid number of arguments"
  $stderr.puts "  usage: ruby #{__FILE__} out.all.itg"
  raise "invalid number of arguments"
end

counts = {}

File.open(ARGV[0]).each do |line|
  trace,_,n = line.chomp.split
  counts[trace] = n.to_i
end

def format_trace(trace)
  c = "\e[42m\e[30mc\e[0m"
  d = "\e[45m\e[30md\e[0m"
  mapped = trace.split(',').map do |i|
    s = sprintf("%06b",i.to_i).gsub('0',c).gsub('1',d)
  end
  mapped.join(" -> ")
end

counts.sort_by {|key,val| -val}.each do |key,val|
  puts "#{format_trace(key)} #{val.to_s.reverse.gsub( /(\d{3})(?=\d)/, '\1,').reverse}"
end

