#include <iostream>
#include "Action.hpp"

char A2C( Action act ) {
  if(act == C) { return 'c'; }
  else if(act == D) { return 'd'; }
  else if(act == U) { return '_'; }
  else if(act == W) { return '*'; }
  return ' ';
}

std::ostream &operator<<(std::ostream &os, const Action &act) {
  os << A2C(act);
  return os;
}

