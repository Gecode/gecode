/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

  template<class View>
  forceinline
  ValSplitMin<View>::ValSplitMin(void) {}
  template<class View>
  forceinline
  ValSplitMin<View>::ValSplitMin(Space& home,
    const ValBranchOptions& vbo)
    : ValSelBase<View,FloatNum>(home,vbo) {}
  template<class View>
  forceinline FloatNum
  ValSplitMin<View>::val(Space&, View x) const {
    return x.med();
  }
  template<class View>
  forceinline ModEvent
  ValSplitMin<View>::tell(Space& home, unsigned int a, View x, FloatNum n) {
    if (a==0)
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.min());
      else return x.lq(home,n);
    }
    else
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.max());
      else return x.gq(home,n);
    }
  }

  template<class View>
  forceinline
  ValSplitMax<View>::ValSplitMax(void) {}
  template<class View>
  forceinline
  ValSplitMax<View>::ValSplitMax(Space& home,
    const ValBranchOptions& vbo)
    : ValSelBase<View,FloatNum>(home,vbo) {}
  template<class View>
  forceinline FloatNum
  ValSplitMax<View>::val(Space&, View x) const {
    return x.med();
  }
  template<class View>
  forceinline ModEvent
  ValSplitMax<View>::tell(Space& home, unsigned int a, View x, FloatNum n) {
    if (a==0)
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.max());
      else return x.gq(home,n);
    }
    else
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.min());
      else return x.lq(home,n);
    }
  }

  template<class View>
  forceinline
  ValSplitRnd<View>::ValSplitRnd(void) {}
  template<class View>
  forceinline
  ValSplitRnd<View>::ValSplitRnd(Space&, const ValBranchOptions& vbo)
    : r(vbo.seed) {}
  template<class View>
  forceinline typename ValSplitRnd<View>::Val
  ValSplitRnd<View>::val(Space&, View x) {
    unsigned int p = r(2);
    return std::pair<FloatNum, bool>(x.med(),(p == 0));
  }
  template<class View>
  forceinline ModEvent
  ValSplitRnd<View>::tell(Space& home, unsigned int a, View x, Val rn) {
    FloatNum& n = rn.first;
    if ( ((a==0) && rn.second) || ((a == 1) && !rn.second) )
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.min());
      else return x.lq(home,n);
    }
    else
    {
      if ((x.min() == n) || (x.max() == n)) return x.eq(home,x.max());
      else return x.gq(home,n);
    }
  }
  template<class View>
  forceinline typename ValSplitRnd<View>::Choice
  ValSplitRnd<View>::choice(Space&) {
    return r;
  }
  template<class View>
  forceinline typename ValSplitRnd<View>::Choice
  ValSplitRnd<View>::choice(const Space&, Archive& e) {
    return Choice(e.get());
  }
  template<class View>
  forceinline void
  ValSplitRnd<View>::commit(Space&, const Choice& c, unsigned int) {
    r = c;
  }
  template<class View>
  forceinline void
  ValSplitRnd<View>::update(Space&, bool, ValSplitRnd<View>& vr) {
    r = vr.r;
  }
  template<class View>
  forceinline void
  ValSplitRnd<View>::dispose(Space&) {}

  template<class View>
  forceinline
  AssignValMin<View>::AssignValMin(void) {}
  template<class View>
  forceinline
  AssignValMin<View>::AssignValMin(Space& home, const ValBranchOptions& vbo)
    : ValSplitMin<View>(home,vbo) {}


  template<class View>
  forceinline
  AssignValMax<View>::AssignValMax(void) {}
  template<class View>
  forceinline
  AssignValMax<View>::AssignValMax(Space& home, const ValBranchOptions& vbo)
    : ValSplitMax<View>(home,vbo) {}


  template<class View>
  forceinline
  AssignValRnd<View>::AssignValRnd(void) {}
  template<class View>
  forceinline
  AssignValRnd<View>::AssignValRnd(Space& home, const ValBranchOptions& vbo)
    : ValSplitRnd<View>(home,vbo) {}

}}}

// STATISTICS: float-branch

