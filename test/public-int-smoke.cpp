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

#include "test/int.hh"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

  bool require(bool condition, const std::string& message) {
    if (!condition) {
      std::cerr << "public-int-smoke: " << message << std::endl;
      return false;
    }
    return true;
  }

  int run_and_capture(std::initializer_list<const char*> args, std::string& output) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (const char* arg : args)
      argv.push_back(const_cast<char*>(arg));

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());
    const int rc = Test::run_registered_tests(static_cast<int>(argv.size()), argv.data());
    std::cout.rdbuf(old);
    output = captured.str();
    return rc;
  }

}

int
main(void) {
  const std::string test_name = "Int::Public::Smoke::Abs";

  std::string list_output;
  if (!require(run_and_capture({"public-int-smoke", "-list"}, list_output) == EXIT_SUCCESS,
               "-list should succeed")) {
    return EXIT_FAILURE;
  }
  if (!require(list_output.find(test_name) != std::string::npos,
               "-list output missing integer smoke test")) {
    return EXIT_FAILURE;
  }
  if (!require(abs_runs == 0,
               "-list should not execute integer smoke tests")) {
    return EXIT_FAILURE;
  }

  std::string run_output;
  if (!require(run_and_capture({"public-int-smoke", "-test", "Int::Public::Smoke::Abs",
                                "-iter", "1", "-stop", "true"},
                               run_output) == EXIT_SUCCESS,
               "filtered integer smoke run should succeed")) {
    return EXIT_FAILURE;
  }
  if (!require(run_output.find(test_name) != std::string::npos,
               "filtered integer smoke run did not print the selected test")) {
    return EXIT_FAILURE;
  }
  if (!require(run_output.find('+') != std::string::npos,
               "filtered integer smoke run did not report success")) {
    return EXIT_FAILURE;
  }
  if (!require(abs_runs > 0,
               "filtered integer smoke run did not execute the helper-backed test")) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

// STATISTICS: test-int
