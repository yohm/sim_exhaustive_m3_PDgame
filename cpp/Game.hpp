#include <array>
//#include <tuple>
#include "Strategy.hpp"

#ifndef GAME_HPP
#define GAME_HPP

typedef std::array<std::array<double,64>,64> umatrix_t;
typedef std::array<double,64> vec64_t;

class Game {
public:
  Game(Strategy _sa, Strategy _sb, Strategy _sc): sa(_sa), sb(_sb), sc(_sc) {};
  ~Game() {};
  FullState Update(FullState fs) const;
  const Strategy sa, sb, sc;
  std::array<double,3> AveragePayoffs(double error, double multi_f, double cost, size_t num_iter=1024) const;
  double CooperationProbability(double error, size_t num_iter=1024, double delta=1.0e-5) const;
  void MakeUMatrix( double e, umatrix_t & m ) const;
  static void MakePayoffVector( double r, double c, vec64_t& va, vec64_t& vb, vec64_t& vc);
  // returns payoff vectors. multiplication factor: r, cost: c
  static vec64_t PowerMethod(const umatrix_t& m, const vec64_t& init_v, size_t num_iter );
  static double Dot(const vec64_t& v1, const vec64_t& v2);
  static double _L1(const vec64_t & v1, const vec64_t& v2);
  static void _MultiplyAndNomalize(const umatrix_t &m, const vec64_t &v, vec64_t &result);
};

#endif //GAME_HPP

