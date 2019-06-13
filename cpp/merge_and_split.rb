require 'pp'

unless ARGV.size == 4
  $stderr.puts "Usage: ruby #{__FILE__} infile_pattern num_input_total num_output output_pattern"
  $stderr.puts "       ruby #{__FILE__} '*/out.passed.[0-9]*' 1486146878 1200 'merged/passed.%05d'"
  raise ArgumentError
end

in_pattern = ARGV[0]
num_input = ARGV[1].to_i
num_output = ARGV[2].to_i
raise "invalid num_output" unless num_output > 1
output_pattern = ARGV[3]

num_lines_per_file = (num_input.to_f / num_output).ceil

line_n = 0
file_count = 0
f = nil

Dir.glob(in_pattern).each do |infile|
  File.open(infile).each_with_index do |line|
    if line_n % num_lines_per_file == 0
      outfile = sprintf(output_pattern, file_count)
      f&.flush; f&.close
      f = File.open(outfile, 'w')
      file_count += 1
    end
    f.print line
    line_n += 1
  end
end

f.flush; f.close

