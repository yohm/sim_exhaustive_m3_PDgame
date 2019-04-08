require 'pp'
require_relative '../lib/graph'
require_relative '../lib/state'
require_relative '../lib/meta_strategy'

S_ALLD = [  # states against AllD
  'cccddd', 'ccdddd', 'cdcddd', 'cddddd',
  'dccddd', 'dcdddd', 'ddcddd', 'dddddd'
]

def construct_strategy(s)
  str_s = '_'*64
  s.each_char.with_index do |a,i|
    s = State.make_from_str(S_ALLD[i])
    str_s[s.to_i] = a
  end
  ms = MetaStrategy.make_from_str(str_s)
end

def dfs(depth, s, &block)
  if depth == s.length
    block.call(s)
  else
    dfs(depth, s+'c', &block)
    dfs(depth, s+'d', &block)
  end
end

defensibles = []
dfs(8, '') do |s|
  str = construct_strategy(s)
  defensibles << str if str.defensible?
end

pp defensibles.count

# check against 'ddd'->'ddc'->'dcd'->'cdd'->'ddd'
defensibles2 = []
defensibles.each do |d|
  # make state change
  s0 = 'dddddd'
  a0 = d.action(s0)
  s1 = "dddddc"
  dfs(3, '') do |acts|
    a1 = acts[0].to_sym
    a2 = acts[1].to_sym
    a3 = acts[2].to_sym
    _d = d.dup
    _d.set(s1, a1)
    s2 = "dd#{a1}dcd"
    _d.set(s2,a2)
    s3 = "d#{a1}#{a2}cdd"
    _d.set(s3,a3)
    defensibles2 << _d if _d.defensible?
  end
end

pp defensibles2.count

defensibles3 = []
# check against 'ddd'->'ddc'->'dcd'->'cdc'->'dcd'->'ddd'
defensibles2.each do |d|
  s0 = "dddddc"
  s1 = "dd#{d.action(s0)}dcd"
  s2 = "d#{d.action(s0)}#{d.action(s1)}cdc" # この状態に対する行動は未定
  dfs(3, '') do |acts|
    a2 = acts[0].to_sym
    a3 = acts[1].to_sym
    a4 = acts[2].to_sym
    _d = d.dup
    _d.set(s2,a2) if _d.action(s2) == :_
    s3 = "#{_d.action(s0)}#{_d.action(s1)}#{_d.action(s2)}dcd"
    _d.set(s3,a3) if _d.action(s3) == :_
    s4 = "#{_d.action(s1)}#{_d.action(s2)}#{_d.action(s3)}cdd"
    _d.set(s4,a4) if _d.action(s4) == :_
    defensibles3 << _d if _d.defensible?
  end
end

pp defensibles3.count
puts defensibles3
