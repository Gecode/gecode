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

#include "test/int.hh"

#include <cstdlib>
#include <iostream>

namespace {

  int abs_runs = 0;

  class AbsSmokeTest : public Test::Int::Test {
  public:
    AbsSmokeTest(void)
      : ::Test::Int::Test("Public::Smoke::Abs",2,-2,2) {}

    bool solution(const ::Test::Int::Assignment& x) const override {
      const int abs_x0 = (x[0] < 0) ? -x[0] : x[0];
      return abs_x0 == x[1];
    }

    void post(Gecode::Space& home, Gecode::IntVarArray& x) override {
      abs_runs++;
      Gecode::abs(home, x[0], x[1]);
    }
  } abs_smoke_test;

}

int
main(int argc, char* argv[]) {
  const int result = Test::run_registered_tests(argc,argv);
  if ((result != EXIT_SUCCESS) || (abs_runs == 0)) {
    std::cerr << "public-int-smoke: helper-backed test did not run"
              << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// STATISTICS: test-int
