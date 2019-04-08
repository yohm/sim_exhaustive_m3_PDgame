require 'pp'
require 'pry'
require_relative '../lib/graph'
require_relative '../lib/state'
require_relative '../lib/meta_strategy'

if ARGV.size == 1
  histo = Hash.new(0)
  File.open(ARGV[0]).each do |l|
    n_ = l.chomp.count('_')
    histo[n_] += 1
  end
  histo.sort_by {|k,v| k}.each do |k,v|
    puts "2^#{k} * #{v}"
  end

  total = histo.map {|k,v| v*2**k }.inject(:+)
  puts "total: #{total}"
end


