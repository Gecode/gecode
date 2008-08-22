/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Int {

  template <class View>
  inline static std::ostream&
  print_view(std::ostream& os, const View& x) {
    if (x.assigned()) {
      return os << x.val();
    } else if (x.range()) {
      return os << '[' << x.min() << ".." << x.max() << ']';
    } else {
      os << '{';
      ViewRanges<View> r(x);
      while (true) {
        if (r.min() == r.max()) {
          os << r.min();
        } else {
          os << r.min() << ".." << r.max();
        }
        ++r;
        if (!r()) break;
        os << ',';
      }
      return os << '}';
    }
  }

  template <class Val, class UnsVal>
  std::ostream&
  print_scale(std::ostream& os, const ScaleView<Val,UnsVal>& x) {
    if (x.assigned()) {
      return os << x.val();
    } else {
      os << '{';
      ViewRanges<ScaleView<Val,UnsVal> > r(x);
      while (true) {
        if (r.min() == r.max()) {
          os << r.min();
        } else {
          os << r.min() << ".." << r.max();
        }
        ++r;
        if (!r()) break;
        os << ',';
      }
      return os << '}';
    }
  }

}}

std::ostream&
operator<<(std::ostream& os, const Gecode::Int::IntView& x) {
  return Gecode::Int::print_view(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::BoolView& x) {
  return Gecode::Int::print_view(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::MinusView& x) {
  return Gecode::Int::print_view(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::OffsetView& x) {
  return Gecode::Int::print_view(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::IntScaleView& x) {
  return Gecode::Int::print_scale<int,unsigned int>(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::DoubleScaleView& x) {
  return Gecode::Int::print_scale<double,double>(os,x);
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::ConstIntView& x) {
  return os << x.val();
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::ZeroIntView&) {
  return os << 0;
}
std::ostream&
operator<<(std::ostream& os, const Gecode::Int::NegBoolView& x) {
  if (x.one())
    return os << 1;
  if (x.zero())
    return os << 0;
  return os << "[0..1]";
}


// STATISTICS: int-var

