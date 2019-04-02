require_relative '../lib/strategy'
require 'pp'

class StrategyEnumerator

  def initialize(pattern)
    @fixed_actions = pattern.chars.map do |c|
      case c
      when 'd'
        :d
      when 'c'
        :c
      when '-'
        nil
      else
        raise StandardError("invalid pattern")
      end
    end
    @fixed_actions.freeze
  end

  def to_bit
    @fixed_actions.map do |act|
      act ? act.to_s : '-'
    end.join
  end

  def size
    num_nil = @fixed_actions.count(nil)
    2 ** num_nil
  end

  def all_strategy
    actions = @fixed_actions.dup

    e = Enumerator.new do |y|

      iterate_for = lambda do |idx|
        possible_actions = []
        if act = @fixed_actions[idx]
          possible_actions = [ act ]
        else
          possible_actions = [:c,:d]
        end
        possible_actions.each do |act|
          actions[idx] = act
          if idx < @fixed_actions.size-1
            iterate_for.call(idx+1)
          else
            y << actions
          end
        end
      end
      iterate_for.call(0)
    end
  end

end

if __FILE__ == $0
  unless ARGV.size == 1
    $stderr.puts "usage: ruby #{__FILE__} pattern"
    raise "invalid argument"
  end
  pattern = ARGV[0]
  se = StrategyEnumerator.new(pattern)
  $stderr.puts se.to_bit, total = se.size
  count = 0
  se.all_strategy.each do |stra|
    $stdout.puts stra.map(&:to_s).join
    count += 1
    if count % 1_000_000 == 0
      $stderr.puts "count: #{count} / #{total}"
    end
  end
  $stdout.flush
end
