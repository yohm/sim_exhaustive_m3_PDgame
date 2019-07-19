require 'pp'

unless ARGV.size == 3
  $stderr.puts "Usage: ruby #{__FILE__} infile_pattern num_output output_pattern"
  $stderr.puts "       ruby #{__FILE__} '*/out.passed.[0-9]*' 1200 'merged/passed.%05d'"
  raise ArgumentError
end

in_pattern = ARGV[0]
num_output = ARGV[1].to_i
raise "invalid num_output" unless num_output > 1
output_pattern = ARGV[2]

IN_FILES = Dir.glob(in_pattern).sort_by {|x| x.split('/')[0].to_i }

lines = IN_FILES.map do |infile|
  $stderr.puts "counting lines : #{infile}"
  `wc -l #{infile}`.split[0].to_i
end
num_input = lines.inject(:+)

num_lines_per_file = (num_input.to_f / num_output).ceil

line_n = 0
file_count = 0
f = nil

IN_FILES.each do |infile|
  $stderr.puts "reading #{infile}"
  File.open(infile).each_with_index do |line|
    if line_n % num_lines_per_file == 0
      outfile = sprintf(output_pattern, file_count)
      $stderr.puts "writing #{outfile}"
      f&.flush; f&.close
      f = File.open(outfile, 'w')
      file_count += 1
    end
    f.print line
    line_n += 1
  end
end

f.flush; f.close

