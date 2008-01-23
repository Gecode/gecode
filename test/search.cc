/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include "gecode/int.hh"
#include "gecode/minimodel.hh"
#include "gecode/search.hh"

#include "test/test.hh"

namespace Test { 

  /// Tests for search engines
  namespace Branch {

    using namespace Gecode;

    /// Values for selecting branchings
    enum HowToBranch {
      HTB_UNARY,  ///< Branch with single alternative
      HTB_BINARY, ///< Branch with two alternatives
      HTB_NARY    ///< Branch with many alternatives
    };

    /// Values for selecting how to constrain
    enum HowToConstrain {
      HTC_LEX_SMALLEST, ///< Constrain for lexically smallest
      HTC_LEX_LARGEST,  ///< Constrain for lexically biggest 
      HTC_BAL_SMALLEST, ///< Constrain for smallest balance
      HTC_BAL_LARGEST   ///< Constrain for largest balance
    };

    /// Values for selecting models
    enum WhichModel {
      WM_FAIL_IMMEDIATE, ///< Model that fails immediately
      WM_FAIL_SEARCH,    ///< Model without solutions
      WM_SOLUTIONS       ///< Model with solutions
    };

    /// Space that immediately fails
    class FailImmediate : public Space {
    public:
      /// Variables used
      IntVarArray x;
      /// Constructor for space creation
      FailImmediate(void) : x(this,1,0,0) {
        rel(this, x[0], IRT_EQ, 1);
      }
      /// Constructor for cloning \a s
      FailImmediate(bool share, FailImmediate& s) : Space(share,s) {
        x.update(this, share, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new FailImmediate(share,*this);
      }
      /// Add constraint for next better solution
      void constrain(Space*) {
      }
    };

    /// Space that requires propagation and has solutions
    class HasSolutions : public Space {
    public:
      /// Variables used
      IntVarArray x;
      /// How to constrain
      HowToConstrain htc;
      /// Branch on \ x according to \a htb
      void branch(const IntVarArgs& x, HowToBranch htb) {
        switch (htb) {
        case HTB_UNARY:
          assign(this, x, INT_ASSIGN_MIN);
          break;
        case HTB_BINARY:
          Gecode::branch(this, x, INT_VAR_NONE, INT_VAL_MIN);
          break;
        case HTB_NARY:
          assign(this, x, INT_ASSIGN_MIN);
          break;
        }
      }
      /// Constructor for space creation
      HasSolutions(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
                   HowToConstrain htc0) 
        : x(this,6,0,5), htc(htc0) {
        distinct(this,x);
        IntVarArgs x1(2); x1[0]=x[0]; x1[1]=x[1]; branch(x1, htb1);
        IntVarArgs x2(2); x2[0]=x[2]; x2[1]=x[2]; branch(x2, htb2);
        IntVarArgs x3(2); x3[0]=x[4]; x3[1]=x[4]; branch(x3, htb3);
      }
      /// Constructor for cloning \a s
      HasSolutions(bool share, HasSolutions& s) 
        : Space(share,s), htc(s.htc) {
        x.update(this, share, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new HasSolutions(share,*this);
      }
      /// Add constraint for next better solution
      void constrain(Space* _s) {
        HasSolutions* s = static_cast<HasSolutions*>(_s);
        switch (htc) {
        case HTC_LEX_SMALLEST:
        case HTC_LEX_LARGEST:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i].init(this, s->x[i].val(), s->x[i].val());
            lex(this, x, (htc == HTC_LEX_SMALLEST) ? IRT_LE : IRT_GR, y);
            break;
          }
        case HTC_BAL_SMALLEST:
        case HTC_BAL_LARGEST:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i].init(this, s->x[i].val(), s->x[i].val());
            IntVar xs(this, -18, 18);
            IntVar ys(this, -18, 18);
            post(this, x[0]+x[1]+x[2]-x[3]-x[4]-x[5] == xs);
            post(this, y[0]+y[1]+y[2]-y[3]-y[4]-y[5] == ys);
            rel(this, 
                abs(this,xs), 
                (htc == HTC_BAL_SMALLEST) ? IRT_LE : IRT_GR,
                abs(this,ys));
            break;
          }
        }
      }
    };

  }

}

// STATISTICS: test-search
