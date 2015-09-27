/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#ifndef __GECODE_SEARCH_META_SEQUENTIAL_PBS_HH__
#define __GECODE_SEARCH_META_SEQUENTIAL_PBS_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Meta { namespace Sequential {

  /// Stop object used for controling slaves in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the slaves
    Stop* so;
    /// Whether search stopped because the slice is done
    bool done;
    /// Number of failures in a slice
    const unsigned int slice;
    /// The current failure limit, incremented for each slice
    unsigned long int l;
  public:
    /// Initialize
    PortfolioStop(const Options& opt);
    /// Return true if portfolio engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
    /// Increment failure limit by slice
    void inc(void);
    /// Whether search was stopped because slice is exhausted
    bool exhausted(void) const;
    /// Disable stop object because a single slave is left
    void disable(void);
  };

  /// Sequential portfolio engine implementation
  template<bool best>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
  protected:
    /// Master statistics
    Statistics stat;
    /// Slave engines
    Engine** slaves;
    /// Number of slave engines
    unsigned int n_slaves;
    /// Current slave to run
    unsigned int cur;
    /// Whether a slave has been stopped
    bool slave_stop;
    /// Stop object
    PortfolioStop* stop;
  public:
    /// Initialize
    PBS(Engine** s, unsigned int n, const Statistics& stat, PortfolioStop* so);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Destructor
    virtual ~PBS(void);
  };

}}}}

#include <gecode/search/meta/sequential/pbs.hpp>

#endif

// STATISTICS: search-meta
