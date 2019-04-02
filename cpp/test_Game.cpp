//
// Created by Yohsuke Murase on 2017/06/06.
//
#include <iostream>
#include "Strategy.hpp"
#include "Game.hpp"

void test_Game() {
  std::cout << "testing Game" << std::endl;
  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy sa(acts), sb(acts), sc(acts);

  Game g(sa,sb,sc);
  FullState initC(C,C,C,C,C,C);
  FullState updated = g.Update( initC );
  std::cout << "  updated from C: " << updated << std::endl;

  FullState initD(D,D,D,D,D,D);
  FullState s = g.Update( initD );
  std::cout << "  updated from D: " << s << std::endl;

  FullState init3(C,D,D,C,D,D);
  FullState s3= g.Update( init3 );
  std::cout << "  updated from init3: " << s3 << std::endl;
}

void test_UMatrix() {
  std::cout << "testing Game" << std::endl;
  const std::array<Action,40> acts = {
      C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C
  };
  Strategy sa(acts), sb(acts), sc(acts);

  Game g(sa,sb,sc);
  umatrix_t m = {0.0};
  g.MakeUMatrix(0.1, m);

  for( auto v : m ) {
    for( double x : v ) {
      printf("%.3f ", x);
    }
    printf("\n");
  }
}

void test_UMatrix2() {
  std::cout << "testing UMatrix2" << std::endl;
  const std::array<Action,40> acts = { // a partially successful strategy
      C,D,D,C,D,D,C,C,D,D,
      C,C,D,D,C,D,D,D,D,D,
      D,D,C,C,D,D,D,C,C,C,
      C,C,D,D,C,D,D,D,D,D,
  };
  Strategy sa(acts), sb(acts), sc(acts);

  Game g(sa,sb,sc);
  umatrix_t m = {0.0};
  g.MakeUMatrix(0.1, m);

  for( auto v : m ) {
    for( double x : v ) {
      printf("%.3f ", x);
    }
    printf("\n");
  }
}

void print_v(vec64_t v) {
  for( auto x : v ) { printf("%.2f ", x); }
  printf("\n");
}

void test_PayoffVector() {
  std::cout << "testing payoff vector" << std::endl;

  double r = 2.0, c = 1.0;
  vec64_t va, vb, vc;
  Game::MakePayoffVector(r,c,va,vb,vc);

  std::cout << "va: ";
  print_v(va);
  std::cout << "vb: ";
  print_v(vb);
  std::cout << "vc: ";
  print_v(vc);
}

void test_PowerMethod() {
  std::cout << "testing payoff vector" << std::endl;
  const std::array<Action,40> acts = {
      C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C
  };
  Strategy sa(acts), sb(acts), sc(acts);

  Game g(sa,sb,sc);
  umatrix_t m = {0.0};
  double error = 0.01;
  g.MakeUMatrix(error, m);
  vec64_t v = {1.0,0.0};
  vec64_t out;

  print_v(v);
  Game::_MultiplyAndNomalize(m, v, out);
  print_v(out);

  vec64_t out2 = Game::PowerMethod(m, v, 100);
  print_v(out2);
  vec64_t out3 = Game::PowerMethod(m, v, 1000);
  print_v(out3);

  double r = 2.0, c = 1.0;
  vec64_t va, vb, vc;
  Game::MakePayoffVector(r,c,va,vb,vc);
  std::cout << "fa : " << Game::Dot(out3,va) << std::endl;
  std::cout << "fb : " << Game::Dot(out3,vb) << std::endl;
  std::cout << "fc : " << Game::Dot(out3,vc) << std::endl;

  auto fs = g.AveragePayoffs(error, r, c );
  std::cout << std::get<0>(fs) << ' ' << std::get<1>(fs) << ' ' << std::get<2>(fs) << std::endl;
}

int main() {
  std::cout << "Testing Game class" << std::endl;

  test_Game();
  test_UMatrix();
  test_UMatrix2();
  test_PayoffVector();
  test_PowerMethod();

  return 0;
}

