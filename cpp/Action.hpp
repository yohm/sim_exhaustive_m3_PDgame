#ifndef ACTION_HPP
#define ACTION_HPP

enum Action {
  C,
  D,
  U, // undetermined
  W  // wild-card
};

char A2C(Action act);
Action C2A(char c);

#endif // ACTION_HPP
