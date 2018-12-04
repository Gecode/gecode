/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Matthias Balzer <matthias.balzer@itwm.fraunhofer.de>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Fraunhofer ITWM, 2017
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

#include <gecode/minimodel.hh>

#ifdef GECODE_HAS_SET_VARS

namespace Gecode {

  /*
   * Operators
   *
   */
  SetRel
  operator ==(const SetExpr& e0, const SetExpr& e1) {
    return SetRel(e0, SRT_EQ, e1);
  }
  SetRel
  operator !=(const SetExpr& e0, const SetExpr& e1) {
    return SetRel(e0, SRT_NQ, e1);
  }
  SetCmpRel
  operator <=(const SetExpr& e0, const SetExpr& e1) {
    return SetCmpRel(e0, SRT_SUB, e1);
  }
  BoolExpr
  operator <=(const SetCmpRel& r, const SetExpr& l) {
    return BoolExpr(r) && BoolExpr(r.r <= l);
  }
  SetCmpRel
  operator >=(const SetExpr& e0, const SetExpr& e1) {
    return SetCmpRel(e0, SRT_SUP, e1);
  }
  BoolExpr
  operator >=(const SetCmpRel& r, const SetExpr& l) {
    return BoolExpr(r) && BoolExpr(r.r >= l);
  }
  SetRel
  operator ||(const SetExpr& e0, const SetExpr& e1) {
    return SetRel(e0, SRT_DISJ, e1);
  }

  namespace {

    /// Boolean expression for IRT relations with SetExpr
    class SetIRTRel : public BoolExpr::Misc {
      /// The set expression
      SetExpr _s;
      /// The integer expression
      LinIntExpr _x;
      /// The integer relation type
      IntRelType _irt;
    public:
      /// Constructor
      SetIRTRel(const SetExpr&, IntRelType, const LinIntExpr&);
      /// Constrain \a b to be equivalent to the expression (negated if \a neg)
      virtual void post(Home, BoolVar b, bool neg,
                        const IntPropLevels&) override;
    };

    SetIRTRel::SetIRTRel(const SetExpr& s, IntRelType irt, const LinIntExpr& x)
      : _s(s), _x(x), _irt(irt) {}

    void
    SetIRTRel::post(Home home, BoolVar b, bool neg,
                    const IntPropLevels& ipls) {
      if (b.zero()) {
        rel(home, _s.post(home), neg ? _irt : Gecode::neg(_irt),
            _x.post(home, ipls));
      } else if (b.one()) {
        rel(home, _s.post(home), neg ? Gecode::neg(_irt) : _irt, 
            _x.post(home, ipls));
      } else {
        rel(home, _s.post(home), neg ? Gecode::neg(_irt) : _irt,
            _x.post(home, ipls), b);
      }
    }
  }


  /*
   * IRT relations with SetExpr
   *
   */
  BoolExpr
  operator ==(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_EQ, y));
  }
  BoolExpr
  operator ==(const LinIntExpr& x, const SetExpr& y) {
    return operator ==(y, x);
  }

  BoolExpr
  operator !=(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_NQ, y));
  }

  BoolExpr
  operator !=(const LinIntExpr& x, const SetExpr& y) {
    return operator !=(y, x);
  }

  BoolExpr
  operator <=(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_LQ, y));
  }

  BoolExpr
  operator <=(const LinIntExpr& x, const SetExpr& y) {
    return operator >=(y, x);
  }

  BoolExpr
  operator <(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_LE, y));
  }

  BoolExpr
  operator <(const LinIntExpr& x, const SetExpr& y) {
    return operator >(y, x);
  }

  BoolExpr
  operator >=(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_GQ, y));
  }

  BoolExpr
  operator >=(const LinIntExpr& x, const SetExpr& y) {
    return operator <=(y, x);
  }

  BoolExpr
  operator >(const SetExpr& x, const LinIntExpr& y) {
    return BoolExpr(new SetIRTRel(x, IRT_GR, y));
  }

  BoolExpr
  operator >(const LinIntExpr& x, const SetExpr& y) {
    return operator <(y, x);
  }

}

#endif

// STATISTICS: minimodel-any
