require 'pp'
require 'pry'
require_relative '../lib/graph'
require_relative '../lib/state'
require_relative '../lib/meta_strategy'

def f(i)
  str = i.to_s.reverse
  str.gsub!(/([0-9]{3})/,"\\1,")
  str.gsub(/,$/,"").reverse
end

if ARGV.size == 1
  histo = Hash.new(0)
  File.open(ARGV[0]).each do |l|
    n_1 = l.chomp.count('_')
    n_2 = l.chomp.count('*')
    histo[n_1+n_2] += 1
  end
  histo.sort_by {|k,v| k}.each do |k,v|
    puts "2^#{k} * #{v}"
  end

  total = histo.map {|k,v| v*2**k }.inject(:+)
  puts "total: #{f(total)}"
elsif ARGV.size > 1
  ARGV.each do |file|
    total = 0
    File.open(file).each do |l|
      n_ = l.chomp.count('_')
      total += 2**n_
    end
    puts "#{file}:\t #{f(total)}"
  end
end


