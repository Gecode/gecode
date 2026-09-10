/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

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
