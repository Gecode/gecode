/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2018
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

  inline
  IntPropLevels::IntPropLevels(IntPropLevel ipl)
    : _linear2(ipl), _linear(ipl), 
      _abs(ipl), 
      _max2(ipl), _max(ipl), _min2(ipl), _min(ipl), 
      _mult(ipl), _div(ipl), _mod(ipl), 
      _sqr(ipl), _sqrt(ipl), _pow(ipl), _nroot(ipl), 
      _element(ipl), 
      _ite(ipl) {}

  inline IntPropLevel
  IntPropLevels::linear2(void) const {
    return _linear2;
  }
  inline IntPropLevels&
  IntPropLevels::linear2(IntPropLevel ipl) {
    _linear2=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::linear(void) const {
    return _linear;
  }
  inline IntPropLevels&
  IntPropLevels::linear(IntPropLevel ipl) {
    _linear=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::abs(void) const {
    return _abs;
  }
  inline IntPropLevels&
  IntPropLevels::abs(IntPropLevel ipl) {
    _abs=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::max2(void) const {
    return _max2;
  }
  inline IntPropLevels&
  IntPropLevels::max2(IntPropLevel ipl) {
    _max2=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::max(void) const {
    return _max;
  }
  inline IntPropLevels&
  IntPropLevels::max(IntPropLevel ipl) {
    _max=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::min2(void) const {
    return _min2;
  }
  inline IntPropLevels&
  IntPropLevels::min2(IntPropLevel ipl) {
    _min2=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::min(void) const {
    return _min;
  }
  inline IntPropLevels&
  IntPropLevels::min(IntPropLevel ipl) {
    _min=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::mult(void) const {
    return _mult;
  }
  inline IntPropLevels&
  IntPropLevels::mult(IntPropLevel ipl) {
    _mult=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::div(void) const {
    return _div;
  }
  inline IntPropLevels&
  IntPropLevels::div(IntPropLevel ipl) {
    _div=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::mod(void) const {
    return _mod;
  }
  inline IntPropLevels&
  IntPropLevels::mod(IntPropLevel ipl) {
    _mod=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::sqr(void) const {
    return _sqr;
  }
  inline IntPropLevels&
  IntPropLevels::sqr(IntPropLevel ipl) {
    _sqr=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::sqrt(void) const {
    return _sqrt;
  }
  inline IntPropLevels&
  IntPropLevels::sqrt(IntPropLevel ipl) {
    _sqrt=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::pow(void) const {
    return _pow;
  }
  inline IntPropLevels&
  IntPropLevels::pow(IntPropLevel ipl) {
    _pow=ipl; return *this;
  }
  inline IntPropLevel
  IntPropLevels::nroot(void) const {
    return _nroot;
  }
  inline IntPropLevels&
  IntPropLevels::nroot(IntPropLevel ipl) {
    _nroot=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::element(void) const {
    return _element;
  }
  inline IntPropLevels&
  IntPropLevels::element(IntPropLevel ipl) {
    _element=ipl; return *this;
  }

  inline IntPropLevel
  IntPropLevels::ite(void) const {
    return _ite;
  }
  inline IntPropLevels&
  IntPropLevels::ite(IntPropLevel ipl) {
    _ite=ipl; return *this;
  }

}

// STATISTICS: minimodel-any

