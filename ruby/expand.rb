require 'pp'

# expand '*' or '_' in the list of strategy sets and print strategies.

unless ARGV.size == 2
  $stderr.puts "invalid number of arguments"
  $stderr.puts "Usage: ruby #{__FILE__} <strategies.txt> <target_depth>"
end

PATTERN = /(\*|_)/

def expand(str, depth)
  if depth == 0
    puts str
    return
  end

  if str =~ PATTERN
    expand( str.sub(PATTERN,'c'), depth-1 )
    expand( str.sub(PATTERN,'d'), depth-1 )
  else
    puts str
  end
end


d = ARGV[1].to_i

File.open(ARGV[0]).each do |l|
  expand(l.chomp, d)
end
