/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004, 2005
 *     Gabor Szokoli, 2004
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

#include "gecode/set.hh"

using namespace Gecode::Set;
using namespace Gecode::Int;

namespace Gecode { namespace Set {

/* 
 * Printing a bound
 *
 */
template <class I>
static void
printBound(std::ostream& os, I& r) {
  os << '{';
  while (r()) {
    if (r.min() == r.max()) {
      os << r.min();
    } else if (r.min()+1 == r.max()) {
      os << r.min() << " " << r.max();
    } else {
      os << r.min() << "#" << r.max();
    }
    ++r;
    if (!r()) break;
    os << ' ';
  }
  os << '}';
}
    
/*
 * Printing a variable or view from the data generaly available
 *
 */
template <class IL, class IU>
static void
printVar(std::ostream& os, const bool assigned, IL& lb, IU& ub,
         unsigned int cardMin, unsigned int cardMax) {
  if (assigned) {
    printBound(os, ub);
    os << "#" << cardMin;
  } else {
    os << "_{";
    printBound(os,lb);
    os << "..";
    printBound(os,ub);
    os << "}";
    if (cardMin==cardMax) {
      os << "#" <<cardMin;
    } else {
      os << "#{" << cardMin << "," << cardMax << "}";
    }
  }
}

}}

std::ostream&
operator<<(std::ostream& os, const SetVarImp& x) {
  LubRanges<SetVarImp*> ub(&x);
  GlbRanges<SetVarImp*> lb(&x);
  printVar(os, x.assigned(), lb, ub, x.cardMin(), x.cardMax()) ;
  return os;
}

std::ostream&
operator<<(std::ostream& os, const SetView& x) {
  LubRanges<SetView> ub(x);
  GlbRanges<SetView> lb(x);
  printVar(os, x.assigned(), lb, ub, x.cardMin(), x.cardMax()) ;
  return os;
}

std::ostream&
operator<<(std::ostream& os, const EmptyView&) {
  return os << "{}#0";
}

std::ostream&
operator<<(std::ostream& os, const UniverseView&) {
  return os << "{" << Gecode::Limits::Set::int_min << "#"
            << Gecode::Limits::Set::int_max << "}#"
            << Gecode::Limits::Set::card_max;
}

std::ostream&
operator<<(std::ostream& os, const ConstantView& s) {
  return os << "{?}#?";
}

std::ostream&
operator<<(std::ostream& os, const SingletonView&) {
  return os << "{?}#1";
}

// STATISTICS: set-var

