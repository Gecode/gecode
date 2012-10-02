/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2012
 *     Vincent Barichard, 2012
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

namespace Gecode {

  forceinline Archive&
  operator <<(Archive& e, std::pair<float,bool> pdb) {
    return e << pdb.first << pdb.second;
  }
  forceinline Archive&
  operator <<(Archive& e, std::pair<double,bool> pdb) {
    return e << pdb.first << pdb.second;
  }

  forceinline Archive&
  operator >>(Archive& e, std::pair<float,bool>& pdb) {
    return e >> pdb.first >> pdb.second;
  }
  forceinline Archive&
  operator >>(Archive& e, std::pair<double,bool>& pdb) {
    return e >> pdb.first >> pdb.second;
  }
}

namespace Gecode { namespace Float { namespace Branch {

  forceinline
  ValSelMed::ValSelMed(Space& home, const ValBranch& vb) 
    : ValSel<FloatView,FloatNum>(home,vb) {}
  forceinline
  ValSelMed::ValSelMed(Space& home, bool shared, ValSelMed& vs)
    : ValSel<FloatView,FloatNum>(home,shared,vs) {}
  forceinline FloatNum
  ValSelMed::val(const Space&, FloatView x, int) {
    return x.med();
  }

  forceinline
  ValSelRnd::ValSelRnd(Space& home, const ValBranch& vb) 
    : ValSel<FloatView,std::pair<FloatNum,bool> >(home,vb), r(vb.rnd()) {}
  forceinline
  ValSelRnd::ValSelRnd(Space& home, bool shared, ValSelRnd& vs)
    : ValSel<FloatView,std::pair<FloatNum,bool> >(home,shared,vs) {
    r.update(home,shared,vs.r);
  }
  forceinline std::pair<FloatNum,bool>
  ValSelRnd::val(const Space&, FloatView x, int) {
    unsigned int p = r(2U);
    return std::pair<FloatNum,bool>(x.med(),(p == 0U));
  }
  forceinline bool
  ValSelRnd::notice(void) const {
    return true;
  }
  forceinline void
  ValSelRnd::dispose(Space&) {
    r.~Rnd();
  }

}}}

// STATISTICS: float-branch

