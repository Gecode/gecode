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

#include "test/test.hh"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

  int passing_runs = 0;
  int failing_runs = 0;

  class PassingSmokeTest : public Test::Base {
  public:
    PassingSmokeTest(void)
      : Test::Base("Smoke::A-Pass") {}

    bool run(void) override {
      passing_runs++;
      return true;
    }
  } passing_smoke_test;

  class FailingSmokeTest : public Test::Base {
  public:
    FailingSmokeTest(void)
      : Test::Base("Smoke::B-Fail") {}

    bool run(void) override {
      failing_runs++;
      return false;
    }
  } failing_smoke_test;

  bool require(bool condition, const std::string& message) {
    if (!condition) {
      std::cerr << "public-runner-smoke: " << message << std::endl;
      return false;
    }
    return true;
  }

  int run_and_capture(std::initializer_list<const char*> args, std::string& output) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (const char* arg : args) {
      argv.push_back(const_cast<char*>(arg));
    }

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
  std::string list_output;
  if (!require(run_and_capture({"public-runner-smoke", "-list"}, list_output) == EXIT_SUCCESS,
               "-list should succeed")) {
    return EXIT_FAILURE;
  }
  const std::string pass_name = "Smoke::A-Pass";
  const std::string fail_name = "Smoke::B-Fail";
  const std::size_t pass_pos = list_output.find(pass_name);
  const std::size_t fail_pos = list_output.find(fail_name);
  if (!require(pass_pos != std::string::npos, "-list output missing pass test")) {
    return EXIT_FAILURE;
  }
  if (!require(fail_pos != std::string::npos, "-list output missing fail test")) {
    return EXIT_FAILURE;
  }
  if (!require(pass_pos < fail_pos, "-list output is not alphabetically ordered")) {
    return EXIT_FAILURE;
  }
  if (!require(passing_runs == 0 && failing_runs == 0,
               "-list should not execute any registered tests")) {
    return EXIT_FAILURE;
  }

  std::string pass_output;
  if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::A-Pass", "-iter", "1", "-stop", "true"},
                               pass_output) == EXIT_SUCCESS,
               "filtered passing run should succeed")) {
    return EXIT_FAILURE;
  }
  if (!require(pass_output.find(pass_name) != std::string::npos,
               "filtered passing run did not print the selected test")) {
    return EXIT_FAILURE;
  }
  if (!require(pass_output.find(fail_name) == std::string::npos,
               "filtered passing run executed an unselected test")) {
    return EXIT_FAILURE;
  }
  if (!require(pass_output.find('+') != std::string::npos,
               "filtered passing run did not report success")) {
    return EXIT_FAILURE;
  }
  if (!require(passing_runs == 1 && failing_runs == 0,
               "filtered passing run counts are wrong")) {
    return EXIT_FAILURE;
  }

  std::string fail_output;
  if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::B-Fail", "-iter", "1", "-stop", "true"},
                               fail_output) == EXIT_FAILURE,
               "filtered failing run should fail")) {
    return EXIT_FAILURE;
  }
  if (!require(fail_output.find(fail_name) != std::string::npos,
               "filtered failing run did not print the selected test")) {
    return EXIT_FAILURE;
  }
  if (!require(fail_output.find("Options: -seed ") != std::string::npos,
               "filtered failing run did not preserve seed diagnostics")) {
    return EXIT_FAILURE;
  }
  if (!require(fail_output.find("-test Smoke::B-Fail") != std::string::npos,
               "filtered failing run did not preserve test diagnostics")) {
    return EXIT_FAILURE;
  }
  if (!require(passing_runs == 1 && failing_runs == 1,
               "filtered failing run counts are wrong")) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

// STATISTICS: test-core
