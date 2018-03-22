/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *     Guido Tack, 2006
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

using namespace Gecode;

/**
 * \brief %Example: Magic sequence
 *
 * Find a magic sequence of length \f$n\f$. A magic sequence of
 * length \f$n\f$ is a sequence \f[x_0,x_1, \ldots, x_{n-1}\f]
 * of integers such that for every \f$i=0,\ldots,n-1\f$:
 *  - \f$x_i\f$ is an integer between \f$0\f$ and \f$n-1\f$.
 *  - the number \f$i\f$ occurs exactly \f$x_i\f$ times in the sequence.
 *
 * See problem 19 at http://www.csplib.org/.
 *
 * Note that "Modeling and Programming with Gecode" uses this example
 * as a case study.
 *
 * \ingroup Example
 *
 */
class MagicSequence : public Script {
private:
  /// Length of sequence
  const int n;
  /// Sequence
  IntVarArray s;
public:
  /// Propagation to use for model
  enum {
    PROP_COUNT,   ///< Use count constraints
    PROP_GCC      ///< Use single global cardinality constraint
  };
  /// The actual model
  MagicSequence(const SizeOptions& opt)
    : Script(opt), n(opt.size()), s(*this,n,0,n-1) {
    switch (opt.propagation()) {
    case PROP_COUNT:
      for (int i=n; i--; )
        count(*this, s, i, IRT_EQ, s[i]);
      linear(*this, s, IRT_EQ, n);
      break;
    case PROP_GCC:
      count(*this, s, s, opt.ipl());
      break;
    }
    linear(*this, IntArgs::create(n,-1,1), s, IRT_EQ, 0);
    branch(*this, s, INT_VAR_NONE(), INT_VAL_MAX());
  }

  /// Constructor for cloning \a e
  MagicSequence(MagicSequence& e) : Script(e), n(e.n) {
    s.update(*this, e.s);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new MagicSequence(*this);
  }
  /// Print sequence
  virtual
  void print(std::ostream& os) const {
    os << "\t";
    for (int i = 0; i<n; i++) {
      os << s[i] << ", ";
      if ((i+1) % 20 == 0)
        os << std::endl << "\t";
    }
    os << std::endl;
  }

};

/** \brief Main-function
 *  \relates MagicSequence
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("MagicSequence");
  opt.solutions(0);
  opt.iterations(4);
  opt.size(500);
  opt.propagation(MagicSequence::PROP_COUNT);
  opt.propagation(MagicSequence::PROP_COUNT,   "count");
  opt.propagation(MagicSequence::PROP_GCC,     "gcc");
  opt.parse(argc,argv);
  Script::run<MagicSequence,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

