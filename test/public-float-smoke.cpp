/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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


#include "test/float.hh"

#include <cstdlib>
#include <iostream>

namespace {

  int float_runs = 0;

  class FloatSmokeTest : public Test::Float::Test {
  public:
    FloatSmokeTest(void)
      : ::Test::Float::Test("Public::Smoke",1,0.0,1.0,1.0,
                            ::Test::Float::CPLT_ASSIGNMENT,false) {}

    ::Test::Float::MaybeType
    solution(const ::Test::Float::Assignment&) const override {
      return ::Test::Float::MT_TRUE;
    }

    void post(Gecode::Space&, Gecode::FloatVarArray&) override {
      float_runs++;
    }
  } float_smoke_test;

}

int
main(int argc, char* argv[]) {
  const int result = Test::run_registered_tests(argc,argv);
  if ((result != EXIT_SUCCESS) || (float_runs == 0)) {
    std::cerr << "public-float-smoke: helper-backed test did not run"
              << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// STATISTICS: test-float
