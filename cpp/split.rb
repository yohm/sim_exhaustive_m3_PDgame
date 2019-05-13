unless ARGV.size == 3
  $stderr.puts "Usage: ruby #{__FILE__} infile num_output output_pattern"
  raise ArgumentError
end

infile = ARGV[0]
num_output = ARGV[1].to_i
raise "invalid num_output" unless num_output > 1
output_pattern = ARGV[2]

num_lines = `wc -l #{infile}`.to_i
num_lines_per_file = (num_lines.to_f / num_output).ceil

file_count = 0
f = nil

File.open(infile).each_with_index do |line,line_n|
  if line_n % num_lines_per_file == 0
    outfile = sprintf(output_pattern, file_count)
    f&.flush; f&.close
    f = File.open(outfile, 'w')
    file_count += 1
  end
  f.print line
end

f.flush; f.close