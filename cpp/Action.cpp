#include <iostream>
#include "Action.hpp"

char A2C( Action act ) {
  return act == C ? 'c' : 'd';
}

std::ostream &operator<<(std::ostream &os, const Action &act) {
  os << A2C(act);
  return os;
}

