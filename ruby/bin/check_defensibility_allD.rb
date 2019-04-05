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
  if depth == 0
    block.call(s)
  else
    dfs(depth-1, s+'c', &block)
    dfs(depth-1, s+'d', &block)
  end
end

defensibles = []
dfs(8, '') do |s|
  str = construct_strategy(s)
  defensibles << str if str.defensible?
end

defensibles.each do |d|
  # make state change
  s0 = 'dddddd'
  a0 = d.action(s0)
  s1 = "dd{a0}ddc"
  dfs(3, '') do |s|
    a1 = s[0]
    d.
  # a1 = d.action(s1)
  # s2 = "d{a0+a1}dcd"
  # a2 = d.action(s2)
  # s3 = "{a0+a1+a2}cdd"
  # a3 = d.action(s3)
  # s4 = "{a1+a2+a3}ddd"
  # (a1,a2,a3)がまだ決まっていない状態。それぞれに対して256通りの戦略が存在する。

  puts d
end
