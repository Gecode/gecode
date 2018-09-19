/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#ifndef __GECODE_SEARCH_RELAX_HH__
#define __GECODE_SEARCH_RELAX_HH__

#include <gecode/kernel.hh>
#include <gecode/search.hh>

namespace Gecode { namespace Search {

  /// Relax variables in \a x from solution \a sx with probability \a p
  template<class VarArgs, class Post>
  forceinline void
  relax(Home home, const VarArgs& x, const VarArgs& sx, Rnd r,
        double p, Post& post);


  template<class VarArgs, class Post>
  forceinline void
  relax(Home home, const VarArgs& x, const VarArgs& sx, Rnd r,
        double p, Post& post) {
    if (home.failed())
      return;
    Region reg;
    // Which variables to assign
    Support::BitSet<Region> ax(reg, static_cast<unsigned int>(x.size()));
    // Select randomly with probability p to relax value
    for (int i=0; i<x.size(); i++)
      if (r(UINT_MAX) >=
          static_cast<unsigned int>(p * static_cast<double>(UINT_MAX)))
        ax.set(static_cast<unsigned int>(i));
    if (ax.all())
      // Choose one variable uniformly and unset it
      ax.clear(r(static_cast<unsigned int>(x.size())));
    for (Iter::Values::BitSet<Support::BitSet<Region> > i(ax); i(); ++i)
      if (post(home, x[i.val()], sx[i.val()]) != ES_OK) {
        home.failed();
        return;
      }
  }

}}

#endif

// STATISTICS: search-other
