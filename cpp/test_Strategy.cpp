#include <iostream>
#include "Strategy.hpp"

void test_State() {
  ShortState s( C, D, 1, -1);
  std::cout << "state: " << s;
  std::cout << "  id: " << s.ID();
  std::cout << "  restored_from_id: " << ShortState::ALL_STATES[ s.ID() ];
  std::cout << std::endl;

  FullState fs(C,D,D,D,C,C);
  std::cout << "fullState: " << fs;
  std::cout << "  toShort: " << fs.ToShortState();
  std::cout << "  id: " << fs.ID();
  std::cout << "  restored_from_id: " << FullState(fs.ID());
  std::cout << std::endl;
  std::cout << "from B: " << fs.FromB();
  std::cout << "  toShortFromB: " << fs.FromB().ToShortState();
  std::cout << "  id: " << fs.FromB().ID();
  std::cout << std::endl;
  std::cout << "from C: " << fs.FromC();
  std::cout << "  toShortFromC: " << fs.FromC().ToShortState();
  std::cout << "  id: " << fs.FromC().ID();
  std::cout << std::endl;

  std::cout << "NumDiff 0=" << fs.NumDiffInT1(fs) << std::endl;
  FullState fs2(C,D,D,C,C,C);
  std::cout << "NumDiff 1=" << fs.NumDiffInT1(fs2) << std::endl;
  FullState fs3(C,D,D,C,C,D);
  std::cout << "NumDiff 2=" << fs.NumDiffInT1(fs3) << std::endl;
  FullState fs4(C,C,D,C,C,D);
  std::cout << "NumDiff 3=" << fs.NumDiffInT1(fs4) << std::endl;
  FullState fs5(D,D,D,D,C,C);
  std::cout << "NumDiff -1=" << fs.NumDiffInT1(fs5) << std::endl;
}

void test_Strategy() {
  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy str(acts);
  std::cout << "strategy:\n" << str << std::endl;
  FullState allC(C,C,C,C,C,C);
  std::cout << " action at:" << allC.ID() << " is " << A2C(str.ActionAt(allC)) << std::endl;
  FullState allD(D,D,D,D,D,D);
  std::cout << " action at:" << allD.ID() << " is " << A2C(str.ActionAt(allD)) << std::endl;
  FullState fs3(C,C,C,C,D,D);
  std::cout << " action at:" << fs3.ID() << " is " << A2C(str.ActionAt(fs3)) << std::endl;

  Strategy str2("ccccddddccccddddccccddddccccddddccccdddd");
  std::cout << "strategy2: \n" << str2 << std::endl;
}

void test_TransitionGraph() {
  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };

  Strategy str(acts);
  Graph g = str.TransitionGraph();
  std::cout << g;

  Graph g2 = str.TransitionGraphWithoutPositiveStates();
  std::cout << g2;

  std::cout << str.ToDot();
}

void test_Defensible1() {
  std::cout << "test_Defensible1" << std::endl;

  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy str(acts);
  std::cout << str << std::endl;
  std::cout << "  is defensible?" << str.IsDefensible1() << std::endl;

  const std::array<Action,40> actsD = {
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D
  };
  Strategy allD(actsD);
  std::cout << allD << std::endl;
  std::cout << "  is defensible?" << allD.IsDefensible1() << std::endl;
}

void test_Defensible() {
  std::cout << "test_Defensible" << std::endl;

  const std::array<Action,40> acts = {
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D,
      C,C,C,C,D,D,D,D
  };
  Strategy str(acts);
  std::cout << str << std::endl;
  std::cout << "  is defensible?" << str.IsDefensible() << std::endl;

  const std::array<Action,40> actsD = {
      C,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D
  };
  Strategy allD(actsD);
  std::cout << allD << std::endl;
  std::cout << "  is defensible?" << allD.IsDefensible() << std::endl;
}

void test_Distinguishable() {
  std::cout << "test_Distinguishable" << std::endl;

  const std::array<Action,40> acts = {
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C
  };
  Strategy allC(acts);
  std::cout << allC << std::endl;
  std::cout << "  is distinguishable? : " << allC.IsDistinguishable() << std::endl;

  const std::array<Action,40> acts2 = {
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D,
      D,D,D,D,D,D,D,D
  };
  Strategy allD(acts2);
  std::cout << allD << std::endl;
  std::cout << "  is distinguishable? : " << allD.IsDistinguishable() << std::endl;

  const std::array<Action,40> acts3 = {
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C,
      C,C,C,C,D,C,C,C,  // DC00=>D
      C,C,C,C,C,C,C,C,
      C,C,C,C,C,C,C,C
  };
  Strategy str(acts3);
  std::cout << str << std::endl;
  std::cout << "  is distinguishable? : " << str.IsDistinguishable() << std::endl;
}

int main() {
  std::cout << "Testing Strategy class" << std::endl;

  test_State();
  test_Strategy();

  test_TransitionGraph();
  test_Defensible1();
  test_Defensible();

  test_Distinguishable();

  return 0;
}

