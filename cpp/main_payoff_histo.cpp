//
// Created by Yohsuke Murase on 2020/02/10.
//

#define _USE_MATH_DEFINES
#include <iostream>
#include <array>
#include <cmath>
#include <cassert>
#include <random>
#include <Eigen/Dense>

class MixedStrategy {
  public:
  MixedStrategy(const std::array<double,64>& p) : prob(p) {};
  const std::array<double,64> prob;
  std::array<double,4> StationaryState(const MixedStrategy& other, double error = 0.0) const {
    Eigen::Matrix<double,64,64> A;
    // construct A
    for(int i=0; i<64; i++) {
      for (int j = 0; j < 64; j++) {
        // transition probability from j to i
        // cooperation prob of A
        if (((j << 1) & 0b110110) != (i & 0b110110)) {
          A(i, j) = 0.0;
        }
        else {
          int j_inv = (((j & 0b111) << 3) | ((j >> 3) & 0b111));  // j from B's viewpoint
          double coop_prob_A = prob[j];
          double coop_prob_B = other.prob[j_inv];
          bool A_c = ((i & 0b001000) == 0); // A cooperates
          double go_target_prob_A = A_c ? coop_prob_A : (1.0 - coop_prob_A);
          bool B_c = ((i & 0b000001) == 0); // B cooperates
          double go_target_prob_B = B_c ? coop_prob_B : (1.0 - coop_prob_B);
          A(i, j) = (1.0 - error) * (1.0 - error) * go_target_prob_A * go_target_prob_B
              + (1.0 - error) * error * go_target_prob_A * (1.0 - go_target_prob_B)
              + (1.0 - error) * error * (1.0 - go_target_prob_A) * go_target_prob_B
              + error * error * (1.0 - go_target_prob_A) * (1.0 - go_target_prob_B);
        }
      }
    }
    for (int i = 0; i < 64; i++) {
      A(i, i) -= 1.0;
    }
    for (int i = 0; i < 64; i++) {
      A(63, i) += 1.0;  // normalization condition
    }
    Eigen::VectorXd b(64);
    for (int i = 0; i < 63; i++) { b(i) = 0.0; }
    b(63) = 1.0;
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    std::array<double, 4> ans = {0.0}; // last action are cc,cd,dc,dd
    for (int i = 0; i < 64; i++) {
      int idx = 0;
      if (i & 0b000001) { idx += 1; }
      if (i & 0b001000) { idx += 2; }
      ans[idx] += x(i);
    }
    return ans;
  }
};

const int R = 3, T = 4, S = 0, P = 1;
const int D = 100;
typedef std::array<std::array<size_t,D>,D> Histo_t;

Histo_t CalcHistoPayoffs(const MixedStrategy& stra, uint64_t seed, size_t n_sample) {
  std::mt19937_64 rnd(seed);
  std::uniform_real_distribution<double> uni(0.0, 1.0);

  std::array<std::array<size_t,D>,D> count;
  for(int i=0; i<D; i++) { for(int j=0; j<D; j++) { count[i][j] = 0; } }

  for(size_t n=0; n<n_sample; n++) {
    if(n % 1000 == 999) { std::cerr << n << " / " << n_sample << std::endl; }
    std::array<double,64> rand_str = {0.0};
    for(int i=0; i<64; i++) { rand_str[i] = uni(rnd); }

    std::array<double,4> stat = stra.StationaryState(rand_str);
    double payoff_a = R*stat[0] + S*stat[1] + T*stat[2] + P*stat[3];
    double payoff_b = R*stat[0] + T*stat[1] + S*stat[2] + P*stat[3];
    int x = (payoff_a / (T-S)) * D;
    int y = (payoff_b / (T-S)) * D;
    assert( x >= 0 && x < D && y >= 0 && y < D );
    count[y][x] += 1;
  }

  return count;
}

int main(int argc, char** argv) {

  const std::array<double,64> CAPRI_P = {
      1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0
  };
  const MixedStrategy CAPRI(CAPRI_P);
  // TFT-ATFT : cdcdcdcddccddccdcdcccdccdccddccdcdcdcdcddccddccdcdcccdccdccddccd
  const std::array<double,64> TFTATFT_P = {
      1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
      0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
      1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
      0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
      1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
      0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
      1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
      0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0
  };
  const MixedStrategy TFTATFT(TFTATFT_P);
  // Generous TFT:
  double p_c = 0.5;
  const std::array<double,64> GTFT_P = {
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c,
      1.0, p_c, 1.0, p_c, 1.0, p_c, 1.0, p_c
  };
  const MixedStrategy GTFT(GTFT_P);

  uint64_t seed = 1234567890;
  int n_sample = atoi(argv[1]);
  Histo_t histo = CalcHistoPayoffs(CAPRI, seed, n_sample);
  for(int i=histo.size()-1; i>=0; i--) {
    for(int j=0; j<histo[i].size(); j++) {
      std::cout << histo[i][j];
      if( j != histo.size() -1 ) { std::cout << ' '; }
    }
    std::cout << '\n';
  }
  return 0;
}
