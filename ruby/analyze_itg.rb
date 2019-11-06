require 'pp'

unless ARGV.size == 1
  $stderr.puts "[error] invalid number of arguments"
  $stderr.puts "  usage: ruby #{__FILE__} out.all.itg"
  raise "invalid number of arguments"
end

counts = Hash.new(0)

Dir.glob(ARGV[0]).each do |f|
  File.open(f).each do |line|
    trace,_,n = line.chomp.split
    counts[trace] += n.to_i
  end
end

def format_trace(trace)
  c = "\e[42m\e[30mc\e[0m"
  d = "\e[45m\e[30md\e[0m"
  mapped = trace.split(',').map do |i|
    if i.to_i == -1
      "\e[44m\e\[30mU\e\[0m"
    else
      s = sprintf("%06b",i.to_i).gsub('0',c).gsub('1',d)
    end
  end
  mapped.join(" -> ")
end

counts.sort_by {|key,val| -val}.each do |key,val|
  puts "#{format_trace(key)} #{val}"
  #puts "#{format_trace(key)} #{val.to_s.reverse.gsub( /(\d{3})(?=\d)/, '\1,').reverse}"
end

