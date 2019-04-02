//
// Created by Yohsuke Murase on 2017/06/26.
//

#include <array>
#include "StrategyM3.hpp"

#ifndef GAME_M3_HPP
#define GAME_M3_HPP

typedef std::array<std::array<double,512>,512> matrix512_t;
typedef std::array<double,512> vec512_t;

class GameM3 {
public:
  GameM3(StrategyM3 _sa, StrategyM3 _sb, StrategyM3 _sc): sa(_sa), sb(_sb), sc(_sc) {};
  ~GameM3() {};
  FullStateM3 Update(const FullStateM3& fs) const;
  const StrategyM3 sa, sb, sc;
  std::array<double,3> AveragePayoffs(double error, double multi_f, double cost, size_t num_iter=1024, double delta=1.0e-5) const;
  double CooperationProbability(double error, size_t num_iter=1024, double delta=1.0e-5) const;
  void MakeUMatrix( double e, matrix512_t & m ) const;
  static void MakePayoffVector( double r, double c, vec512_t& va, vec512_t& vb, vec512_t& vc);
  // returns payoff vectors. multiplication factor: r, cost: c
  static vec512_t PowerMethod(const matrix512_t& m, const vec512_t& init_v, size_t num_iter );
  static double Dot(const vec512_t& v1, const vec512_t& v2);
  static double _L1(const vec512_t & v1, const vec512_t& v2);
  static void _MultiplyAndNormalize(const matrix512_t &m, const vec512_t &v, vec512_t &result);

  static const size_t N; // = 512
};

#endif //GAME_M3_HPP

