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

#ifndef __GECODE_SEARCH_SEQ_PBS_HH__
#define __GECODE_SEARCH_SEQ_PBS_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// Shared stop information
  class SharedStopInfo {
  public:
    /// Whether search stopped because the slice is done
    bool done;
    /// The current failure limit, incremented for each slice
    unsigned long int l;
  };

  /// Stop object used for controling slaves in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the slaves
    Stop* so;
    /// Pointer to shared stop information
    SharedStopInfo* ssi;
  public:
    /// Initialize
    PortfolioStop(Stop* so);
    /// Intialize shared stop information
    void share(SharedStopInfo* ssi);
    /// Return true if portfolio engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
  };

  /// Runnable slave of a portfolio master
  class Slave {
  protected:
    /// The slave engine
    Engine* slave;
    /// Stop object
    Stop* stop;
  public:
    /// Initialize with slave \a s and its stop object \a so
    void init(Engine* s, Stop* so);
    /// Return next solution
    Space* next(void);
    /// Return statistics of slave
    Statistics statistics(void) const;
    /// Check whether slave has been stopped
    bool stopped(void) const;
    /// Constrain with better solution \a b
    void constrain(const Space& b);
    /// Perform one run
    void run(void);
    /// Delete slave
    ~Slave(void);
  };

  /// Sequential portfolio engine implementation
  template<bool best>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
  protected:
    /// Master statistics
    Statistics stat;
    /// Shared slave information
    SharedStopInfo ssi;
    /// Size of a slice
    unsigned int slice;
    /// Slaves
    Slave* slaves;
    /// Number of slave engines
    unsigned int n_slaves;
    /// Current slave to run
    unsigned int cur;
    /// Whether a slave has been stopped
    bool slave_stop;
  public:
    /// Initialize
    PBS(Engine** slaves, Stop** stops, unsigned int n,
        const Statistics& stat, const Search::Options& opt);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Constrain future solutions to be better than \a b
    virtual void constrain(const Space& b);
    /// Destructor
    virtual ~PBS(void);
  };

}}}

#include <gecode/search/seq/pbs.hpp>

#endif

// STATISTICS: search-seq
