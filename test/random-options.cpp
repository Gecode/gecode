// Copyright (c) 2026 Mikael Zayenz Lagerkvist. MIT license; see LICENSE.
#include <gecode/driver.hh>
#ifdef TEST_RANDOM_FLATZINC
#include <gecode/flatzinc.hh>
#endif
#include <iostream>

template<class Options>
int check(int argc, char* argv[]) {
  Options opt("random-options");
  opt.parse(argc,argv);
  if (argc != 1)
    return 2;
  auto first = opt.rnd();
  auto second = opt.rnd();
  std::cout << first.state() << '\n';
  for (int i=0; i<8; ++i) {
    const auto draw = first(UINT64_MAX);
    if (draw != second(UINT64_MAX))
      return 3;
    std::cout << draw << '\n';
  }
  return 0;
}

int main(int argc, char* argv[]) {
#ifdef TEST_RANDOM_FLATZINC
  if (argc > 1 && std::string(argv[1]) == "flatzinc")
    return check<Gecode::FlatZinc::FlatZincOptions>(argc-1,argv+1);
#endif
  return check<Gecode::Options>(argc,argv);
}
