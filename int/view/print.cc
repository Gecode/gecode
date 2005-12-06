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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "int.hh"

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
operator<<(std::ostream& os, const Gecode::Int::NegBoolView& x) {
  if (x.one())
    return os << 1;
  if (x.zero())
    return os << 0;
  return os << "[0..1]";
}


// STATISTICS: int-var

