/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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
#include <gecode/set.hh>

using namespace Gecode;

/**
 * \brief %Example: %Steiner triples
 *
 * See also problem 044 at http://www.csplib.org/.
 *
 * \ingroup Example
 *
 */
class Steiner : public Script {
public:
  /// Model variants
  enum {
    MODEL_NONE,   ///< Use simple relation constraint
    MODEL_MATCHING, ///< Use matching constraints
    MODEL_SEQ ///< Use sequence constraints
  };
  /// Order of the Steiner problem
  int n;
  /// Number of Steiner triples
  int noOfTriples;

  /// The steiner triples
  SetVarArray triples;

  /// Actual model
  Steiner(const SizeOptions& opt)
    : Script(opt), n(opt.size()), noOfTriples((n*(n-1))/6),
      triples(*this, noOfTriples, IntSet::empty, 1, n, 3U, 3U) {

    for (int i=0; i<noOfTriples; i++) {
      for (int j=i+1; j<noOfTriples; j++) {
        SetVar x = triples[i];
        SetVar y = triples[j];

        SetVar atmostOne(*this,IntSet::empty,1,n,0U,1U);
        rel(*this, (x & y) == atmostOne);

        IntVar x1(*this,1,n);
        IntVar x2(*this,1,n);
        IntVar x3(*this,1,n);
        IntVar y1(*this,1,n);
        IntVar y2(*this,1,n);
        IntVar y3(*this,1,n);

        if (opt.model() == MODEL_NONE) {
          /* Naive alternative:
           * just including the ints in the set
           */
          rel(*this, singleton(x1) <= x);
          rel(*this, singleton(x2) <= x);
          rel(*this, singleton(x3) <= x);
          rel(*this, singleton(y1) <= y);
          rel(*this, singleton(y2) <= y);
          rel(*this, singleton(y3) <= y);

        } else if (opt.model() == MODEL_MATCHING) {
          /* Smart alternative:
           * Using matching constraints
           */

          channelSorted(*this, {x1,x2,x3}, x);
          channelSorted(*this, {y1,y2,y3}, y);
        } else if (opt.model() == MODEL_SEQ) {
          SetVar sx1 = expr(*this, singleton(x1));
          SetVar sx2 = expr(*this, singleton(x2));
          SetVar sx3 = expr(*this, singleton(x3));
          SetVar sy1 = expr(*this, singleton(y1));
          SetVar sy2 = expr(*this, singleton(y2));
          SetVar sy3 = expr(*this, singleton(y3));
          sequence(*this,{sx1,sx2,sx3},x);
          sequence(*this,{sy1,sy2,sy3},y);
        }

        /* Breaking symmetries */
        rel(*this, x1 < x2);
        rel(*this, x2 < x3);
        rel(*this, x1 < x3);

        rel(*this, y1 < y2);
        rel(*this, y2 < y3);
        rel(*this, y1 < y3);

        linear(*this,
               {(n+1)*(n+1), n+1, 1,  -(n+1)*(n+1), -(n+1), -1},
               {x1,          x2,  x3, y1,           y2,     y3},
               IRT_LE, 0);
      }
    }

    branch(*this, triples, SET_VAR_NONE(), SET_VAL_MIN_INC());
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int i=0; i<noOfTriples; i++) {
      os << "\t[" << i << "] = " << triples[i] << std::endl;
    }
  }
  /// Constructor for copying \a s
  Steiner(Steiner& s) : Script(s), n(s.n), noOfTriples(s.noOfTriples) {
    triples.update(*this, s.triples);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Steiner(*this);
  }
};

/** \brief Main-function
 *  \relates Steiner
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Steiner");
  opt.model(Steiner::MODEL_NONE);
  opt.model(Steiner::MODEL_NONE, "rel", "Use simple relation constraints");
  opt.model(Steiner::MODEL_MATCHING, "matching", "Use matching constraints");
  opt.model(Steiner::MODEL_SEQ, "sequence", "Use sequence constraints");
  opt.size(9);
  opt.iterations(20);
  opt.parse(argc,argv);
  Script::run<Steiner,DFS,SizeOptions>(opt);
  return 0;
}


// STATISTICS: example-any

