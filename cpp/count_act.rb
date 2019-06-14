require 'pp'

unless ARGV.size >= 1
  $stderr.puts "Usage: ruby #{__FILE__} infiles"
  raise ArgumentError
end

histo_c = Array.new(64,0)
histo_d = Array.new(64,0)

ARGV.each do |arg|
  Dir.glob(arg).sort.each do |infile|
    $stderr.puts "reading #{infile}"
    File.open(infile).each_with_index do |line,idx|
      $stderr.puts "  line : #{idx}" if idx % 100000 == 0
      str = line.chomp.split[0]
      num_w = str.count('*')
      num = 2 ** num_w
      raise unless str.size == 64
      64.times do |i|
        case str[i] 
        when 'c'
          histo_c[i] += num
        when 'd'
          histo_d[i] += num
        when '*'
          histo_c[i] += num/2
          histo_d[i] += num/2
        else
          raise "must not happen"
        end
      end
    end
  end
end

64.times do |i|
  puts "#{i} #{histo_c[i]} #{histo_d[i]}"
end
