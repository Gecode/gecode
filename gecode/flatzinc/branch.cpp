/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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


#include <gecode/flatzinc/branch.hh>

namespace Gecode { namespace FlatZinc {

  void
  PosIntChoice::archive(Archive& e) const {
    Choice::archive(e);
    e << _pos;
    e << _val;
  }

  void
  branch(Home home, const IntVarArgs& x, const BoolVarArgs& y,
         IntBoolVarBranch vars, IntValBranch vals) {
    if (home.failed()) return;
    ViewArray<Int::IntView> xv(home,x);
    ViewArray<Int::BoolView> yv(home,y);
    ValSelCommitBase<Int::IntView,int>* xvsc =
      Int::Branch::valselcommit(home,vals);
    ValSelCommitBase<Int::BoolView,int>* yvsc =
      Int::Branch::valselcommit(home,i2b(vals));
    switch (vars.select()) {
    case IntBoolVarBranch::SEL_AFC_MAX:
      {
        MeritMaxAFC m(home,vars);
        IntBoolBrancher<MeritMaxAFC>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_ACTION_MAX:
      {
        MeritMaxAction m(home,vars);
        IntBoolBrancher<MeritMaxAction>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_CHB_MAX:
      {
        MeritMaxCHB m(home,vars);
        IntBoolBrancher<MeritMaxCHB>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_AFC_SIZE_MAX:
      {
        MeritMaxAFCSize m(home,vars);
        IntBoolBrancher<MeritMaxAFCSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_ACTION_SIZE_MAX:
      {
        MeritMaxActionSize m(home,vars);
        IntBoolBrancher<MeritMaxActionSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_CHB_SIZE_MAX:
      {
        MeritMaxCHBSize m(home,vars);
        IntBoolBrancher<MeritMaxCHBSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    default:
      GECODE_NEVER;
    }
  }

}}

// STATISTICS: flatzinc-branch

