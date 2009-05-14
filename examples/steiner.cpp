/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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
#include <gecode/set.hh>

using namespace Gecode;

/**
 * \brief %Example: Steiner triples
 *
 * See also problem 044 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
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
    : n(opt.size()), noOfTriples((n*(n-1))/6),
      triples(*this, noOfTriples, IntSet::empty, 1, n, 3, 3) {

    for (int i=0; i<noOfTriples; i++) {
      for (int j=i+1; j<noOfTriples; j++) {
        SetVar x = triples[i];
        SetVar y = triples[j];

        SetVar atmostOne(*this,IntSet::empty,1,n,0,1);
        cardinality(*this, atmostOne,0,1);
        rel(*this, x, SOT_INTER, y, SRT_EQ, atmostOne);

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
          rel(*this, x, SRT_SUP, x1);
          rel(*this, x, SRT_SUP, x2);
          rel(*this, x, SRT_SUP, x3);
          rel(*this, y, SRT_SUP, y1);
          rel(*this, y, SRT_SUP, y2);
          rel(*this, y, SRT_SUP, y3);

        } else  if (opt.model() == MODEL_MATCHING) {
          /* Smart alternative:
           * Using matching constraints
           */

          IntVarArgs xargs(3);
          xargs[0] = x1; xargs[1] = x2; xargs[2] = x3;
          channel(*this, xargs, x);

          IntVarArgs yargs(3);
          yargs[0] = y1; yargs[1] = y2; yargs[2] = y3;
          channel(*this, yargs, y);
        } else if (opt.model() == MODEL_SEQ) {
          SetVar tmp20(*this,IntSet::empty,1,n);
          SetVar tmp21(*this,IntSet::empty,1,n);
          SetVar tmp22(*this,IntSet::empty,1,n);
          SetVar tmp23(*this,IntSet::empty,1,n);
          SetVar tmp24(*this,IntSet::empty,1,n);
          SetVar tmp25(*this,IntSet::empty,1,n);
          rel(*this, tmp20, SRT_EQ, x1);
          rel(*this, tmp21, SRT_EQ, x2);
          rel(*this, tmp22, SRT_EQ, x3);
          rel(*this, tmp23, SRT_EQ, y1);
          rel(*this, tmp24, SRT_EQ, y2);
          rel(*this, tmp25, SRT_EQ, y3);
          SetVarArgs xargs(3);
          xargs[0] = tmp20; xargs[1] = tmp21; xargs[2] = tmp22;
          SetVarArgs yargs(3);
          yargs[0] = tmp23; yargs[1] = tmp24; yargs[2] = tmp25;
          sequence(*this,xargs,x);
          sequence(*this,yargs,y);
        }

        /* Breaking symmetries */

        rel(*this, x1,IRT_LE,x2);
        rel(*this, x2,IRT_LE,x3);
        rel(*this, x1,IRT_LE,x3);

        rel(*this, y1,IRT_LE,y2);
        rel(*this, y2,IRT_LE,y3);
        rel(*this, y1,IRT_LE,y3);

        IntArgs ia(6,(n+1)*(n+1),n+1,1,-(n+1)*(n+1),-(n+1),-1);
        IntVarArgs iva(6);
        iva[0]=x1; iva[1]=x2; iva[2]=x3;
        iva[3]=y1; iva[4]=y2; iva[5]=y3;
        linear(*this, ia, iva, IRT_LE, 0);
      }
    }

    branch(*this, triples, SET_VAR_NONE, SET_VAL_MIN_INC);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int i=0; i<noOfTriples; i++) {
      os << "\t[" << i << "] = " << triples[i] << std::endl;
    }
  }

  /// Constructor for copying \a s
  Steiner(bool share, Steiner& s) : Script(share,s), n(s.n), noOfTriples(s.noOfTriples) {
    triples.update(*this, share, s.triples);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Steiner(share,*this);
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

