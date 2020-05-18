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

class ReactiveStrategy {
  public:
  ReactiveStrategy(double _p, double _q) : p(_p), q(_q) {};
  double p, q;
  int Type(double R, double T, double S, double P, double epsilon) const { // 0: other, 1: partner, 2: rival
    // q*=min{1−(T−R)∕(R−S),(R−P)∕(T−P)}
    double q_star = std::min( 1.0-(T-R)/(R-S), (R-P)/(T-P) );
    double partner_d = 1.0 - p;
    if( q > q_star ) {
      partner_d = std::sqrt( (1.0-p)*(1.0-p) + (q-q_star)*(q-q_star) );
    }
    if( q < epsilon ) {
      if( partner_d < epsilon ) {
        return ( q <= partner_d ) ? 2 : 1;
      }
      else return 2;
    }
    else {
      if( partner_d < epsilon ) { return 1; }
    }
    return 0;
  }
  static std::pair<double,double> PartnerRivalFractions(double R, double T, double S, double P, double epsilon) {
    double q_star = std::min( 1.0-(T-R)/(R-S), (R-P)/(T-P) );
    if( q_star < epsilon ) { throw std::runtime_error("q_star is smaller than epsilon"); }
    double partner_area = 0.5 * epsilon * epsilon + (q_star-epsilon) * epsilon + M_PI * epsilon * epsilon * 0.25;
    double rival_area = (1.0-epsilon) * epsilon + 0.5 * epsilon * epsilon;
    return std::make_pair(partner_area, rival_area);
  }
  std::array<double,4> StationaryState(const ReactiveStrategy& other, double error = 0.0) const {
    Eigen::Matrix<double,4,4> A;
    // state 0: cc, 1: cd, 2: dc, 3: dd
    // calculate transition probability from j to i
    for(int j=0; j<4; j++) {
      double c_A = (j == 0 || j == 2) ? p : q;
      double c_B = (j == 0 || j == 1) ? other.p : other.q;
      // taking into account error
      c_A = (1.0-error) * c_A + error * (1.0 - c_A);
      c_B = (1.0-error) * c_B + error * (1.0 - c_B);
      A(0,j) = c_A * c_B;
      A(1,j) = c_A * (1.0-c_B);
      A(2,j) = (1.0-c_A) * c_B;
      A(3,j) = (1.0-c_A) * (1.0-c_B);
    }
    for(int i=0; i<4; i++) {
      A(i,i) -= 1.0;
    }
    for(int i=0; i<4; i++) {
      A(3,i) += 1.0;  // normalization condition
    }
    Eigen::VectorXd b(4);
    for(int i=0; i<4; i++) { b(i) = 0.0;}
    b(3) = 1.0;
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);
    std::array<double,4> ans = {0};
    for(int i=0; i<4; i++) {
      ans[i] = x(i);
    }
    return ans;
  }
};

class ReactiveStrategyOrCAPRI {
  public:
  ReactiveStrategyOrCAPRI(int _is_capri_ta, double _p, double _q) : is_capri_ta(_is_capri_ta), reactive(_p,_q), stat_with_self_valid(false) {};
  int is_capri_ta; // 0: reactive, 1: capri, 2: tft-atft
  ReactiveStrategy reactive;
  std::array<double,4> stat_with_self;
  bool stat_with_self_valid;
  int Type(double R, double T, double S, double P, double epsilon) const { // 0: other, 1: partner, 2: rival, 3: capri, 4: tft-atft
    if( is_capri_ta == 1 ) { return 3; }
    else if( is_capri_ta == 2 ) { return 4; }
    else {
      return reactive.Type(R,T,S,P,epsilon);
    }
  }
  std::array<double,4> StationaryStateWithSelf(double error) {
    if(!stat_with_self_valid) {
      stat_with_self = StationaryState(*this, error);
      stat_with_self_valid = true;
    }
    return stat_with_self;
  }
  std::array<double,4> StationaryState(const ReactiveStrategyOrCAPRI& other, double error) const {
    if( is_capri_ta == 0 && other.is_capri_ta == 0 ) {
      return reactive.StationaryState(other.reactive, error);
    }
    else {
      // CAPRI : cdddcdddcdcddddddcddcdddddddddddcdcdcdcdddddddddddddcdccddddddcd
      const double CAPRI_P[64] = {
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0
      };
      // TFT-ATFT : cdcdcdcddccddccdcdcccdccdccddccdcdcdcdcddccddccdcdcccdccdccddccd
      const double TFTATFT_P[64] = {
        1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0
      };
      Eigen::Matrix<double,64,64> A;
      // construct A
      for(int i=0; i<64; i++) {
        for (int j = 0; j < 64; j++) {
          // transition probability from j to i
          // cooperation prob of A
          if (((j << 1) & 0b110110) != (i & 0b110110)) {
            A(i, j) = 0.0;
          } else {
            int j_inv = (((j & 0b111) << 3) | ((j >> 3) & 0b111));  // j from B's viewpoint
            double coop_prob_A;
            if (is_capri_ta == 1) { coop_prob_A = CAPRI_P[j]; }
            else if( is_capri_ta == 2) { coop_prob_A = TFTATFT_P[j]; }
            else {
              coop_prob_A = ((j & 1) == 0) ? reactive.p : reactive.q;
            }
            double coop_prob_B;
            if (other.is_capri_ta == 1) {
              coop_prob_B = CAPRI_P[j_inv];
            }
            else if( other.is_capri_ta == 2) {
              coop_prob_B = TFTATFT_P[j_inv];
            }
            else {
              coop_prob_B = ((j_inv & 1) == 0) ? other.reactive.p : other.reactive.q;
            }

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
  }

};

class Ecosystem {
  public:
  Ecosystem(uint64_t seed) : resident(0,0.0,0.0), rnd(seed) {};
  ReactiveStrategyOrCAPRI resident;
  void UpdateResident(double R, double T, double S, double P, uint64_t N, double sigma, double e, double capri_rate, double tft_atft_rate, size_t discrete_level) {
    ReactiveStrategyOrCAPRI mutant = Mutant(capri_rate, tft_atft_rate, discrete_level);
    double rho = FixationProb(R,T,S,P,N,sigma,e,mutant);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    if( uni(rnd) < rho ) {
      resident = mutant;
    }
  }
  void MeasureAvgRho(double R, double T, double S, double P, size_t n_samples) {
    double ave_s_xx = 0.0, ave_s_xy = 0.0, ave_s_yx = 0.0;
    for(size_t i = 0; i<n_samples; i++) {
      double e = 0.1;
      // auto mut = Mutant(0.0, 0.0, 10);
      auto mut = ReactiveStrategyOrCAPRI(0, 1.0, 0.5);
      auto res = ReactiveStrategyOrCAPRI(2, 0.0, 0.0);
      const auto xx = mut.StationaryStateWithSelf(e);
      const double s_xx = xx[0] * R + xx[1] * S + xx[2] * T + xx[3] * P;
      const auto xy = mut.StationaryState(res, e);
      const double s_xy = xy[0] * R + xy[1] * S + xy[2] * T + xy[3] * P;
      const double s_yx = xy[0] * R + xy[2] * S + xy[1] * T + xy[3] * P;
      const auto yy = res.StationaryStateWithSelf(e);
      const double s_yy = yy[0] * R + yy[1] * S + yy[2] * T + yy[3] * P;
      std::cerr << s_xx << ' ' << s_xy << ' ' << s_yx << ' ' << s_yy << std::endl;
      std::cerr << (s_yy - s_xx) << ' ' << (s_yx - s_xy) << ' ' << (s_yy - s_xy) << std::endl;

      size_t N = 50;
      double sigma = 10.0 / N;
      double rho_inv = 0.0;
      for(int i=0; i<N; i++) {
        double x = sigma * i * 0.5 * ( (2*N-3-i)*s_yy + (i+1)*s_yx - (2*N-1-i)*s_xy - (i-1)*s_xx );
        rho_inv += std::exp(x);
      }
      double rho = 1.0 / rho_inv;
      std::cerr << rho << std::endl;
      ave_s_xx += s_xx;
      ave_s_xy += s_xy;
      ave_s_yx += s_yx;
    }
    std::cerr << ave_s_xx/n_samples << ' ' << ave_s_xy/n_samples << ' ' << ave_s_yx/n_samples << std::endl;
  }
  private:
  ReactiveStrategyOrCAPRI Mutant(double capri_rate, double tft_atft_rate, size_t discrete_level) {
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    double r = uni(rnd);
    if( r < capri_rate ) {
      return ReactiveStrategyOrCAPRI(1, 0.0, 0.0);
    }
    else if( r < capri_rate + tft_atft_rate) {
      return ReactiveStrategyOrCAPRI(2, 0.0, 0.0);
    }
    else {
      if(discrete_level > 0) {
        auto dis_uni = std::uniform_int_distribution<size_t>(0,discrete_level);
        double d = static_cast<double>(discrete_level);
        return ReactiveStrategyOrCAPRI(0, dis_uni(rnd)/d, dis_uni(rnd)/d);
      }
      else {
        return ReactiveStrategyOrCAPRI(0, uni(rnd), uni(rnd));
      }
    }
  }
  double FixationProb(double R, double T, double S, double P, int N, double sigma, double e, ReactiveStrategyOrCAPRI& mutant) {
    const auto xx = mutant.StationaryStateWithSelf(e);
    const double s_xx = xx[0] * R + xx[1] * S + xx[2] * T + xx[3] * P;
    const auto xy = mutant.StationaryState(resident, e);
    const double s_xy = xy[0] * R + xy[1] * S + xy[2] * T + xy[3] * P;
    const double s_yx = xy[0] * R + xy[2] * S + xy[1] * T + xy[3] * P;
    const auto yy = resident.StationaryStateWithSelf(e);
    const double s_yy = yy[0] * R + yy[1] * S + yy[2] * T + yy[3] * P;
    // \frac{1}{\rho} = \sum_{i=0}^{N-1} \exp\left( \sigma \sum_{j=1}^{i} \left[(N-j-1)s_{yy} + js_{yx} - (N-j)s_{xy} - (j-1)s_{xx} \right] \right) \\
    //                = \sum_{i=0}^{N-1} \exp\left( \frac{\sigma i}{2} \left[(-i+2N-3)s_{yy} + (j+1)s_{yx} - (-i+2N-1)s_{xy} - (i-1)s_{xx} \right] \right)
    double rho_inv = 0.0;
    for(int i=0; i<N; i++) {
      double x = sigma * i * 0.5 * ( (2*N-3-i)*s_yy + (i+1)*s_yx - (2*N-1-i)*s_xy - (i-1)*s_xx );
      rho_inv += std::exp(x);
    }
    return 1.0 / rho_inv;
  }
  std::mt19937_64 rnd;
};

int main(int argc, char** argv) {
  if( argc != 11 ) {
    std::cerr << "Error : invalid argument" << std::endl;
    std::cerr << "  Usage : " << argv[0] << " <benefit> <cost> <N> <N_sigma> <error rate> <tmax> <capri_rate> <tft_atft_rate> <discrete_level> <rand_seed>" << std::endl;
    return 1;
  }

  double benefit = std::strtod(argv[1], nullptr);
  double cost = std::strtod(argv[2], nullptr);

  double R = benefit - cost;
  double T = benefit;
  double S = -cost;
  double P = 0;
  uint64_t N = std::strtoull(argv[3], nullptr,0);
  double N_sigma = std::strtod(argv[4], nullptr);
  double sigma = N_sigma / N;
  double e = std::strtod(argv[5], nullptr);
  uint64_t tmax = std::strtoull(argv[6], nullptr,0);
  double capri_rate = std::strtod(argv[7], nullptr);
  double tft_atft_rate = std::strtod(argv[8], nullptr);
  size_t discrete_level = std::strtoul(argv[9], nullptr, 0);
  uint64_t seed = std::strtoull(argv[10], nullptr,0);
  double epsilon = 0.099;

  Ecosystem eco(seed);
  // eco.MeasureAvgRho(R,T,S,P, 1000);
  // return 0;
  uint64_t partner_count = 0, rival_count = 0;
  uint64_t capri_count = 0, tft_atft_count = 0;
  uint64_t t_int = 10000;
  double coop_rate = 0.0;
  for(uint64_t t = 0; t < tmax; t++) {
    eco.UpdateResident(R,T,S,P,N,sigma,e,capri_rate, tft_atft_rate, discrete_level);
    int type = eco.resident.Type(R,T,S,P,epsilon);
    if( type == 1 ) { partner_count++; }
    else if( type == 2 ) { rival_count++; }
    else if( type == 3 ) { capri_count++; }
    else if( type == 4 ) { tft_atft_count++; }

    coop_rate += eco.resident.StationaryState( eco.resident, e )[0];
    // std::cerr << eco.resident.reactive.p << ' ' << eco.resident.reactive.q << std::endl;

    if( t % t_int == t_int - 1) {
      double other = static_cast<double>(t - partner_count - rival_count - capri_count - tft_atft_count) / t;
      double partner = static_cast<double>(partner_count) / t;
      double rival = static_cast<double>(rival_count) / t;
      double capri = static_cast<double>(capri_count) / t;
      double ta = static_cast<double>(tft_atft_count) / t;
      std::cerr << t << ' ' << other << ' ' << partner << ' ' << rival << ' ' << capri << ' ' << ta << ' ' << coop_rate/t << std::endl;
    }
  }

  auto fractions = ReactiveStrategy::PartnerRivalFractions(R,T,S,P,epsilon);
  double other = static_cast<double>(tmax - partner_count - rival_count - capri_count - tft_atft_count) / tmax;
  double partner = static_cast<double>(partner_count) / tmax;
  double rival = static_cast<double>(rival_count) / tmax;
  double capri = static_cast<double>(capri_count) / tmax;
  double ta = static_cast<double>(tft_atft_count) / tmax;
  double other_f = static_cast<double>(tmax - partner_count - rival_count) / tmax / ((1.0-capri_rate-tft_atft_rate)*(1.0 - fractions.first - fractions.second));
  double partner_f = static_cast<double>(partner_count) / tmax / ((1.0-capri_rate-tft_atft_rate)*fractions.first);
  double rival_f = static_cast<double>(rival_count) / tmax / ((1.0-capri_rate-tft_atft_rate)*fractions.second);
  double capri_f = (capri_count > 0) ? static_cast<double>(capri_count) / tmax / capri_rate : 0.0;
  double ta_f = (tft_atft_count > 0) ? static_cast<double>(tft_atft_count) / tmax / tft_atft_rate : 0.0;
  std::cout << "{\"other\":" << other << ", "
            << "\"partner\":" << partner << ", "
            << "\"rival\":" << rival << ", "
            << "\"capri\":" << capri << ", "
            << "\"tft_atft\":" << ta << ", "
            << "\"other_frac\":" << other_f << ", "
            << "\"partner_frac\":" << partner_f << ", "
            << "\"rival_frac\":" << rival_f << ", "
            << "\"capri_frac\":" << capri_f << ", "
            << "\"tft_atft_frac\":" << ta_f << ", "
            << "\"cooperation_rate\":" << coop_rate/tmax << " }" << std::endl;

  return 0;
}
