/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <gecode/search/seq/pbs.hh>

namespace Gecode { namespace Search { namespace Seq {

  Stop*
  pbsstop(Stop* so) {
    return new PortfolioStop(so);
  }

  Engine*
  pbsengine(Engine** slaves, Stop** stops, unsigned int n_slaves,
            const Statistics& stat, const Search::Options& opt, bool best) {
    if (best)
      return new PBS<true>(slaves,stops,n_slaves,stat,opt);
    else
      return new PBS<false>(slaves,stops,n_slaves,stat,opt);
  }

}}}

#ifdef GECODE_HAS_THREADS

#include <gecode/search/par/pbs.hh>

namespace Gecode { namespace Search { namespace Par {

  Stop*
  pbsstop(Stop* so) {
    return new PortfolioStop(so);
  }

  Engine*
  pbsengine(Engine** slaves, Stop** stops, unsigned int n_slaves,
         const Statistics& stat, bool best) {
    if (best)
      return new PBS<CollectBest>(slaves,stops,n_slaves,stat);
    else
      return new PBS<CollectAll>(slaves,stops,n_slaves,stat);
  }

}}}

#endif

// STATISTICS: search-other
