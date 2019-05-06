require_relative 'meta_strategy'

h = Hash.new(0)
h2 = Hash.new(0)

def return_to_0_from_a_neighbor(g, tc)
  neighbors = []
  tc.each do |n|
    neighbors << (n^1)  # xor
    neighbors << (n^8)
  end

  reach_0 = false
  neighbors.each do |ngb|
    histo = []  # trace from the neighbor
    n = ngb
    until histo.include?(n)
      histo.push(n)
      break if g.links[n].size != 1
      n = g.links[n][0]
    end
    if n == 0
      reach_0 = true
      break
    end
  end

  reach_0
end

def trace(g, ini)
  histo = []
  n = ini
  until histo.include?(n)
    histo.push(n)
    break if g.links[n].size != 1
    n = g.links[n][0]
  end
  histo
end


def c2_states(g)
  c0 = [0]
  c1 = c0.map {|s| [s^1,s^8] }.flatten.uniq
  c1 = c1.map do |n|
    trace(g, n)
  end.flatten.uniq
  pp c1
  c2 = c1.map {|s| [s^1,s^8] }.flatten.uniq
  c2 = c2.map do |n|
    trace(g, n)
  end.flatten.uniq
  c2
end


counts = {not_fixed: 0, is_efficient: 0, cannot_judge: 0}

File.open(ARGV[0]).each_with_index do |l,i|
  if i % 1000 == 0
    $stderr.puts i
  end
  s = MetaStrategy.make_from_str(l.chomp)
  g = s.transition_graph_with_self
  tcs = g.terminanl_components
  s_tcs = tcs.map {|c| c.map {|i| sprintf("%06b",i) } }
  pp s_tcs
  all_fixed = true
  tcs.all? do |tc|
    if tc.any? {|n| g.links[n].size > 1 } # out degree > 1
      $stdout.puts "not fixed component: #{tc.inspect}"
      all_fixed = false
    end
  end
  if all_fixed
    $stdout.puts "terminanl components are all fixed"
    if tcs.all? {|tc| return_to_0_from_a_neighbor(g, tc) }
      $stdout.puts "  return to 0"
      counts[:is_efficient] += 1
    else
      $stdout.puts "  cannot judge efficiency"
      counts[:cannot_judge] += 1
      $stdout.puts tcs.inspect, tcs.map {|tc| return_to_0_from_a_neighbor(g, tc) }.inspect
      $stdout.puts c2_states(g).sort.inspect
      #$stdout.puts g.inspect
      # g.to_dot($stdout)
      break
    end
  else
    counts[:not_fixed] += 1
  end
  # m = s_tcs.map {|c| c.size }.max
  # h[m] += 1
  # cs.each do |c|
  #   if c.size <= 4
  #     h2[c] += 1
  #   end
  # end
  # 
  # すべてのterminal componentsに対して
  #   neighborからstate-0に戻るパスが存在する
  #   -> efficiencyが確定
end

pp counts
