require 'pp'

unless ARGV.size == 2
  $stderr.puts "invalid number of arguments"
  $stderr.puts "Usage: ruby #{__FILE__} <strategies.txt> <target_depth>"
end

PATTERN = /\*/

def expand(str, depth)
  if depth == 0
    puts str
    return
  end

  expand( str.sub(PATTERN,'c'), depth-1 )
  expand( str.sub(PATTERN,'d'), depth-1 )
end


d = ARGV[1].to_i

File.open(ARGV[0]).each do |l|
  l = l.chomp
  n_1 = l.count('_')
  if n_1 > 0
    if n_1 > d
      raise "n_1 > #{d} is found"
    end
    puts l
  else
    n_2 = l.count('*')
    if n_2 > d
      expand(l, n_2-d)
    else
      puts l
    end
  end
end
