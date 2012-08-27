/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <iomanip>

using namespace Gecode;

/**
 * \brief %Example: Finding optimal %Golomb rulers
 *
 * The script makes use of two lower bounds taken from:
 *   Barbara Smith, Kostas Stergiou, Toby Walsh,
 *   Modelling the Golomb Ruler Problem.
 *   In IJCAI 99 Workshop on Non-binary Constraints,
 *   1999.
 *
 * See also problem 6 at http://www.csplib.org/.
 *
 * The upper bound used is from the trivial construction where
 * distances between consecutive marks are increasing powers of two.
 *
 * Note that "Modeling and Programming with Gecode" uses this example
 * as a case study.
 *
 * \ingroup Example
 *
 */
class GolombRuler : public MinimizeScript {
protected:
  /// Array for ruler marks
  IntVarArray m;
public:
  /// Search variants
  enum {
    SEARCH_BAB,    ///< Use branch and bound to optimize
    SEARCH_RESTART ///< Use restart to optimize
  };
  /// Actual model
  GolombRuler(const SizeOptions& opt)
    : m(*this,8,0,8) {

    dom(*this, m[0], 0,1);
    dom(*this, m[1], 0,1);
    dom(*this, m[2], IntSet(IntArgs(4, 0,1,2,4)));
    dom(*this, m[3], IntSet(IntArgs(3, 2,3,4)));
    dom(*this, m[4], IntSet(IntArgs(3, 2,3,4)));
    dom(*this, m[5], IntSet(IntArgs(2, 4,6)));
    dom(*this, m[6], 4,8);
    dom(*this, m[7], 7,8);

    distinct(*this, m, ICL_DOM);
  }

  /// Return cost
  virtual IntVar cost(void) const {
    return m[m.size()-1];
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tm[" << m.size() << "] = " << m << std::endl;
  }

  /// Constructor for cloning \a s
  GolombRuler(bool share, GolombRuler& s)
    : MinimizeScript(share,s) {
    m.update(*this, share, s.m);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new GolombRuler(share,*this);
  }
};

/** \brief Main-function
 *  \relates GolombRuler
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("GolombRuler");
  opt.solutions(0);
  opt.size(10);
  opt.icl(ICL_BND);
  opt.search(GolombRuler::SEARCH_BAB);
  opt.search(GolombRuler::SEARCH_BAB, "bab");
  opt.search(GolombRuler::SEARCH_RESTART, "restart");
  opt.parse(argc,argv);
  if (opt.size() > 0)
    switch (opt.search()) {
    case GolombRuler::SEARCH_BAB:
      MinimizeScript::run<GolombRuler,BAB,SizeOptions>(opt); break;
    case GolombRuler::SEARCH_RESTART:
      MinimizeScript::run<GolombRuler,Restart,SizeOptions>(opt); break;
    }
  return 0;
}

// STATISTICS: example-any

