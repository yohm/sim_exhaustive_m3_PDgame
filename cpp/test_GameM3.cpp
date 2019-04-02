//
// Created by Yohsuke Murase on 2017/06/26.
//
#include <iostream>
#include "StrategyM3.hpp"
#include "GameM3.hpp"


void test_PowerMethod() {
  std::cerr << __func__ << std::endl;
  const char* acts =
      "cdcdcdcdddccdddccccdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddccddcdddcdcdddcddcdddcddddddddddcdddcdddddcddd"
          "cddcdddcdddddddddddccdddcdcccddcddcccccdccddccddccddcdddcddcdddcddcdcccdccddccddccccddccddccddcdddddddcddddddd"
          "ddcdcccdccddcdddcddddddcddddddddddddcdcdcdcddddcdddccdcdcdcddcdddcddcdcdcdcddddcdddccdcdcdcddcdddcddcdddcddddd"
          "cdddcddccddcddddddddddcdddcdddddcdddcddcdddcddddddddddddcdddcddcdddcddcdcccdccddccddccddcdddcddcdddcddcdcccdcc"
          "ddccddccccddccddcdddcdddddddddddddddddddccddccddcdddcddddddddddddddddddd";
  const char* allc =
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
  StrategyM3 sa(acts), sb(acts), sc(allc);

  GameM3 g(sa,sb,sc);

  double error = 0.0005;
  double r = 2.0, c = 1.0;
  auto fs = g.AveragePayoffs(error, r, c, 8192, 1.0e-5 );
  std::cout << std::get<0>(fs) << ' ' << std::get<1>(fs) << ' ' << std::get<2>(fs) << std::endl;
}

int main() {
  test_PowerMethod();

  return 0;
}

