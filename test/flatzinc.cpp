/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2014
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

#include "test/flatzinc.hh"

namespace Test { namespace FlatZinc {

  FlatZincTest::FlatZincTest(const std::string& name, const std::string& source,
                             const std::string& expected, bool allSolutions)
    : Base("FlatZinc::"+name), _name(name), _source(source), _expected(expected),
      _allSolutions(allSolutions) {}

  bool
  FlatZincTest::run(void) {
    using namespace Gecode;
    Support::Timer t_total;
    t_total.start();
    Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
    fznopt.allSolutions(_allSolutions);
    Gecode::FlatZinc::Printer p;
    Gecode::FlatZinc::FlatZincSpace* fg = NULL;
    try {
      std::stringstream ss(_source);
      fg = Gecode::FlatZinc::parse(ss, p, olog);

      if (fg) {
        fg->createBranchers(p, fg->solveAnnotations(), fznopt,
                            false, olog);
        fg->shrinkArrays(p);
        std::ostringstream os;
        fg->run(os, p, fznopt, t_total);

        if (_expected == os.str()) {
          return true;
        } else {
          if (opt.log)
            olog << "FlatZinc produced the following output:\n" << os.str() << "\n";
          return false;
        }
      } else {
        if (opt.log)
          olog << "Could not parse input\n";
        return false;
      }
      delete fg;
    } catch (Gecode::FlatZinc::Error& e) {
      if (opt.log)
        olog << ind(2) << "FlatZinc error : " << e.toString() << std::endl;
      return false;
    }
    return true;
  }

}}

// STATISTICS: test-flatzinc
