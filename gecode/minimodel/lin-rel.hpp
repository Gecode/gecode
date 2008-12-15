/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

  /*
   * Operations for linear expressions
   *
   */
  template<class Var>
  forceinline
  LinRel<Var>::LinRel(void) {}

  template<class Var>
  forceinline
  LinRel<Var>::LinRel(const LinExpr<Var>& l, IntRelType irt0, 
                      const LinExpr<Var>& r)
    : e(l-r), irt(irt0) {}

  template<class Var>
  forceinline
  LinRel<Var>::LinRel(const LinExpr<Var>& l, IntRelType irt0, int r)
    : e(l-r), irt(irt0) {}

  template<class Var>
  forceinline
  LinRel<Var>::LinRel(int l, IntRelType irt0, const LinExpr<Var>& r)
    : e(l-r), irt(irt0) {}

  template<class Var>
  forceinline IntRelType
  LinRel<Var>::neg(IntRelType irt) {
    switch (irt) {
    case IRT_EQ: return IRT_NQ;
    case IRT_NQ: return IRT_EQ;
    case IRT_LQ: return IRT_GR;
    case IRT_LE: return IRT_GQ;
    case IRT_GQ: return IRT_LE;
    case IRT_GR: return IRT_LQ;
    default: GECODE_NEVER;
    }
    return IRT_LQ;
  }

  template<class Var>
  inline void
  LinRel<Var>::post(Space& home, bool t, IntConLevel icl) const {
    if (t) {
      e.post(home,irt,icl);
    } else {
      e.post(home,neg(irt),icl);
    }
  }

  template<class Var>
  forceinline void
  LinRel<Var>::post(Space& home, const BoolVar& b, bool t,
                    IntConLevel icl) const {
    if (t) {
      e.post(home,irt,b,icl);
    } else {
      e.post(home,neg(irt),b,icl);
    }
  }


  /*
   * Construction of linear relations
   *
   */
  inline LinRel<IntVar>
  operator ==(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }
  inline LinRel<IntVar>
  operator ==(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_EQ,r);
  }

  inline LinRel<IntVar>
  operator !=(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }
  inline LinRel<IntVar>
  operator !=(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_NQ,r);
  }

  inline LinRel<IntVar>
  operator <(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }
  inline LinRel<IntVar>
  operator <(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LE,r);
  }

  inline LinRel<IntVar>
  operator <=(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }
  inline LinRel<IntVar>
  operator <=(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_LQ,r);
  }

  inline LinRel<IntVar>
  operator >(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }
  inline LinRel<IntVar>
  operator >(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GR,r);
  }

  inline LinRel<IntVar>
  operator >=(int l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(int l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const IntVar& l, int r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const LinExpr<IntVar>& l, int r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const IntVar& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const IntVar& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const LinExpr<IntVar>& l, const IntVar& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }
  inline LinRel<IntVar>
  operator >=(const LinExpr<IntVar>& l, const LinExpr<IntVar>& r) {
    return LinRel<IntVar>(l,IRT_GQ,r);
  }



  inline LinRel<BoolVar>
  operator ==(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }
  inline LinRel<BoolVar>
  operator ==(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_EQ,r);
  }

  inline LinRel<BoolVar>
  operator !=(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }
  inline LinRel<BoolVar>
  operator !=(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_NQ,r);
  }

  inline LinRel<BoolVar>
  operator <(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }
  inline LinRel<BoolVar>
  operator <(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LE,r);
  }

  inline LinRel<BoolVar>
  operator <=(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }
  inline LinRel<BoolVar>
  operator <=(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_LQ,r);
  }

  inline LinRel<BoolVar>
  operator >(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }
  inline LinRel<BoolVar>
  operator >(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GR,r);
  }

  inline LinRel<BoolVar>
  operator >=(int l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(int l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const BoolVar& l, int r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const LinExpr<BoolVar>& l, int r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const BoolVar& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const BoolVar& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const LinExpr<BoolVar>& l, const BoolVar& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }
  inline LinRel<BoolVar>
  operator >=(const LinExpr<BoolVar>& l, const LinExpr<BoolVar>& r) {
    return LinRel<BoolVar>(l,IRT_GQ,r);
  }


  /*
   * Posting
   *
   */

  template<class Var>
  forceinline void
  post(Space& home, const LinRel<Var>& r, IntConLevel icl) {
    if (home.failed()) return;
    r.post(home,true,icl);
  }
  forceinline void
  post(Space& home, bool r, IntConLevel) {
    if (home.failed()) return;
    if (!r)
      home.fail();
  }

}

// STATISTICS: minimodel-any
