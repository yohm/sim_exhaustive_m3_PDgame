# count number of strategies from a list of strategy sets
#
# usage: ruby count.rb init_dede.*
#   this will count the number of strategies in files that match the pattern.

def f(i)
  str = i.to_s.reverse
  str.gsub!(/([0-9]{3})/,"\\1,")
  str.gsub(/,$/,"").reverse
end

all_total = 0
ARGV.each do |a|
  Dir.glob(a).each do |file|
    total = 0
    File.open(file).each do |l|
      str,n_rej = l.chomp.split
      n_w = str.count('*') + str.count('_')
      num = 2**n_w - n_rej.to_i
      total += num
    end
    puts "#{file}:\t #{f(total)}"
    all_total += total
  end
end

puts f(all_total)
