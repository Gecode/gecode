/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

namespace Test {

  /*
   * Commandline options
   *
   */
  inline
  Options::Options(void)
    : seed(0), iter(defiter), fixprob(deffixprob), stop(true), log(false)
  {}

  /*
   * Base class for tests
   *
   */
  inline const std::string&
  Base::name(void) const {
    return _name;
  }
  inline Base*
  Base::tests(void) {
    return _tests;
  }
  inline Base*
  Base::next(void) const {
    return _next;
  }
  inline void
  Base::next(Base* n) {
    _next=n;
  }
  inline bool
  Base::fixpoint(void) {
    return fixpoint(_rand);
  }

  inline bool
  Base::fixpoint(Gecode::Support::RandomGenerator& rand) {
    return rand(opt.fixprob) == 0;
  }

  inline std::string
  Base::str(bool b) {
    std::stringstream s;
    if (b)
      s << "+";
    else
      s << "-";
    return s.str();
  }

  inline std::string
  Base::str(int i) {
    std::stringstream s;
    s << i;
    return s.str();
  }

  inline std::string
  Base::str(const Gecode::IntArgs& x) {
    std::string s = "";
    for (int i=0; i<x.size()-1; i++)
      s += str(x[i]) + ",";
    return "[" + s + str(x[x.size()-1]) + "]";
  }


}

// STATISTICS: test-core
