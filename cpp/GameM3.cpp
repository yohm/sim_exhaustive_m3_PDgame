//
// Created by Yohsuke Murase on 2017/06/26.
//

#include <iostream>
#include <cmath>
#include "GameM3.hpp"

const size_t GameM3::N = 512;

FullStateM3 GameM3::Update(const FullStateM3& fs) const {
  Action act_a = sa.ActionAt( fs );
  Action act_b = sb.ActionAt( fs.FromB() );
  Action act_c = sc.ActionAt( fs.FromC() );
  return fs.NextState(act_a,act_b,act_c);
}

void GameM3::MakeUMatrix( double e, matrix512_t &m) const {
  for( size_t i=0; i<N; i++ ) {
    FullStateM3 si(i);
    for( size_t j=0; j<N; j++) {
      // calculate the transition probability from j to i
      FullStateM3 sj(j);
      FullStateM3 next = Update(sj);
      int d = next.NumDiffInT1(si);
      if( d == -1 ) { m[i][j] = 0.0; }               // when j->i cannot happen
      else if( d == 3 ) { m[i][j] = e*e*e; }         // when j->i happens with O(e^3)
      else if( d == 2 ) { m[i][j] = e*e*(1.0-e); }   // when j->i happens with O(e^2)
      else if( d == 1 ) { m[i][j] = e*(1.0-e)*(1.0-e); } // when j->i happens with O(e)
      else { m[i][j] = (1.0-e)*(1.0-e)*(1.0-e); } // when next(j)==i
    }
  }
}

void GameM3::MakePayoffVector(double r, double cost, vec512_t &va, vec512_t &vb, vec512_t &vc) {
  for( size_t i=0; i<N; i++) {
    FullStateM3 fs(i);
    const Action a = fs.a_1, b = fs.b_1, c = fs.c_1;
    size_t num_C = 0;
    if( a == C ) { num_C++;}
    if( b == C ) { num_C++;}
    if( c == C ) { num_C++;}
    double g = num_C * cost * r / 3.0;

    va[i] = ( a == C ) ? g-cost : g;
    vb[i] = ( b == C ) ? g-cost : g;
    vc[i] = ( c == C ) ? g-cost : g;
  }
}

void GameM3::_MultiplyAndNormalize(const matrix512_t &m, const vec512_t &v, vec512_t &result) {
  double sum = 0.0;
  for( size_t i=0; i<N; i++ ) {
    result[i] = 0.0;
    for( size_t j=0; j<N; j++ ) {
      result[i] += m[i][j] * v[j];
    }
    sum += result[i];
  }
  for( size_t i=0; i<N; i++ ) { result[i] /= sum; }
}

vec512_t GameM3::PowerMethod(const matrix512_t &m, const vec512_t &init_v, size_t num_iter) {
  vec512_t va = init_v, vb = {0.0};
  for( size_t i=0; i<num_iter/2; i++) {
    _MultiplyAndNormalize(m, va, vb);
    _MultiplyAndNormalize(m, vb, va);
  }
  return va;
}

double GameM3::Dot(const vec512_t &v1, const vec512_t &v2) {
  double d = 0.0;
  for( size_t i=0; i<N; i++) {
    d += v1[i] * v2[i];
  }
  return d;
}

double GameM3::_L1(const vec512_t &v1, const vec512_t &v2) {
  double d = 0.0;
  for( size_t i=0; i<N; i++) {
    d += fabs(v1[i] - v2[i]);
  }
  return d;
}

std::array<double,3> GameM3::AveragePayoffs(double error, double multi_f, double cost, size_t num_iter, double delta) const {
  matrix512_t m;
  MakeUMatrix(error, m);
  vec512_t init = { 0.0 };
  for( auto& x : init ) { x = 1.0 / N; }
  vec512_t out = GameM3::PowerMethod(m, init, num_iter);
  double l1 = _L1(init,out);

  vec512_t va, vb, vc;
  GameM3::MakePayoffVector(multi_f, cost, va, vb, vc);

  while( l1 > delta ) {
    init = out;
    out = GameM3::PowerMethod(m, init, num_iter);
    l1 = _L1(init,out);
    double fa = GameM3::Dot(out, va);
    std::cerr << "iterating: " << l1 << " " << out[0] << " " << fa << std::endl;
  }
#ifdef DEBUG
  for( auto x: out ) {
    std::cerr << x << std::endl;
  }
#endif

  double fa = GameM3::Dot(out, va);
  double fb = GameM3::Dot(out, vb);
  double fc = GameM3::Dot(out, vc);
  std::array<double,3> ret = {fa,fb,fc};
  return ret;
}

double GameM3::CooperationProbability(double error, size_t num_iter, double delta) const {
  matrix512_t m;
  MakeUMatrix(error, m);
  vec512_t init = { 0.0 };
  for( auto& x : init ) { x = 1.0 / N; }
  vec512_t out = GameM3::PowerMethod(m, init, num_iter);
  double l1 = _L1(init,out);
  while( l1 > delta ) {
    init = out;
    out = GameM3::PowerMethod(m, init, num_iter);
    l1 = _L1(init,out);
    std::cerr << "iterating : " << l1 << std::endl;
  }

  return out[0];
}

