/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include "gecode/set.hh"
#include "examples/support.hh"

/**
 * \brief %Example: Steiner triples
 *
 * This is problem 044 from csplib.
 *
 * \ingroup ExProblem
 *
 */
class Steiner : public Example {
public:

  int n;
  int n1;
  int n1n1;
  int len;

  SetVarArray root;

  Steiner(const SizeOptions& opt)
    : n(opt.size()),
      n1(n+1), n1n1(n1*n1), len((n*(n-1))/6),
      root(this,len, IntSet::empty, 1, n, 3, 3) {

    for (int i=0; i<len; i++) {
      for (int j=i+1; j<len; j++) {
        SetVar x = root[i];
        SetVar y = root[j];

        SetVar atmostOne(this,IntSet::empty,1,n,0,1);
        cardinality(this, atmostOne,0,1);
        rel(this, x, SOT_INTER, y, SRT_EQ, atmostOne);

        IntVar x1(this,1,n);
        IntVar x2(this,1,n);
        IntVar x3(this,1,n);
        IntVar y1(this,1,n);
        IntVar y2(this,1,n);
        IntVar y3(this,1,n);

        //        if (opt.naive) {
        if (true) {
        
          /* Naive alternative:
           * just including the ints in the set
           */
          rel(this, x, SRT_SUP, x1);
          rel(this, x, SRT_SUP, x2);
          rel(this, x, SRT_SUP, x3);
          rel(this, y, SRT_SUP, y1);
          rel(this, y, SRT_SUP, y2);
          rel(this, y, SRT_SUP, y3);
        } else {

          /* Smart alternative:
           * Using matching constraints
           */
        
          IntVarArgs xargs(3);
          xargs[0] = x1; xargs[1] = x2; xargs[2] = x3;
          match(this, x,xargs);
        
          IntVarArgs yargs(3);
          yargs[0] = y1; yargs[1] = y2; yargs[2] = y3;
          match(this, y,yargs);
        
        }
        
        /* Breaking symmetries */
        
        rel(this, x1,IRT_LE,x2);
        rel(this, x2,IRT_LE,x3);
        rel(this, x1,IRT_LE,x3);
        
        rel(this, y1,IRT_LE,y2);
        rel(this, y2,IRT_LE,y3);
        rel(this, y1,IRT_LE,y3);
        
        IntArgs ia(6,n1n1,n1,1,-n1n1,-n1,-1);
        IntVarArgs iva(6);
        iva[0]=x1; iva[1]=x2; iva[2]=x3;
        iva[3]=y1; iva[4]=y2; iva[5]=y3;
        linear(this, ia, iva, IRT_LE, 0);
      }
    }


    branch(this, root, SETBVAR_NONE, SETBVAL_MIN);
  }

  Steiner(bool share, Steiner& s) : Example(share,s),
                                    n(s.n), n1(s.n1), n1n1(s.n1n1), len(s.len)
  {
    root.update(this, share, s.root);
  }

  virtual Space*
  copy(bool share) {
    return new Steiner(share,*this);
  }

  virtual void
  print(void) {
    for (int i=0; i<len; i++) {
      std::cout << "\t[" << i << "]" << root[i] << std::endl;
    }
  }

};

int
main(int argc, char* argv[]) {
  SizeOptions opt("Steiner");
  opt.size(9);
  //  opt.naive = true;
  opt.iterations(20);
  opt.parse(argc,argv);
  Example::run<Steiner,DFS,SizeOptions>(opt);
  return 0;
}


// STATISTICS: example-any

