/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
 *     Vincent Barichard, 2012
 *
 *  Last modified:
 *     $Date: 2012-03-27 13:23:23 +0200 (Tue, 27 Mar 2012) $ by $Author: schulte $
 *     $Revision: 12634 $
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
   * Floating point value: member functions
   *
   */
  forceinline
  NewFloatVal::NewFloatVal(void) {}
  forceinline
  NewFloatVal::NewFloatVal(const FloatNum& n) : x(n) {}
  forceinline
  NewFloatVal::NewFloatVal(const FloatNum& l, const FloatNum& u) : x(l,u) {}
  forceinline
  NewFloatVal::NewFloatVal(const FloatVal& i) : x(i) {}
  forceinline
  NewFloatVal::NewFloatVal(const NewFloatVal& v) : x(v.x) {}

  forceinline NewFloatVal&
  NewFloatVal::operator =(const FloatNum& n) {
    x = n; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator =(const NewFloatVal& v) {
    x = v.x; return *this;
  }
    
    //    void assign(FloatNum const &l, FloatNum const &u);

  forceinline const FloatNum&
  NewFloatVal::lower(void) const {
    return x.lower();
  }
  forceinline const FloatNum&
  NewFloatVal::upper(void) const {
    return x.upper();
  }
    
  forceinline NewFloatVal
  NewFloatVal::empty(void) {
    NewFloatVal e(FloatVal::empty()); return e;
  }
  forceinline NewFloatVal
  NewFloatVal::whole(void) {
    NewFloatVal w(FloatVal::whole()); return w;
  }
  forceinline NewFloatVal
  NewFloatVal::hull(FloatNum x, FloatNum y) {
    NewFloatVal h(FloatVal::hull(x,y)); return h;
  }
    
  forceinline NewFloatVal&
  NewFloatVal::operator +=(const FloatNum& n) {
    x += n; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator -=(const FloatNum& n) {
    x -= n; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator *=(const FloatNum& n) {
    x *= n; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator /=(const FloatNum& n) {
    x /= n; return *this;
  }

  forceinline NewFloatVal&
  NewFloatVal::operator +=(const NewFloatVal& v) {
    x += v.x; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator -=(const NewFloatVal& v) {
    x -= v.x; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator *=(const NewFloatVal& v) {
    x *= v.x; return *this;
  }
  forceinline NewFloatVal&
  NewFloatVal::operator /=(const NewFloatVal& v) {
    x /= v.x; return *this;
  }

}

// STATISTICS: float-var

