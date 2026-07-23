/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2015
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_SEARCH_SEQ_PBS_HH
#define GECODE_SEARCH_SEQ_PBS_HH

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// Shared stop information
  class SharedStopInfo {
  public:
    /// Whether search stopped because the slice is done
    bool done;
    /// The current failure limit, incremented for each slice
    unsigned long long int l;
  };

  /// Stop object used for controlling variants in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the variants
    Stop* so;
    /// Pointer to shared stop information
    SharedStopInfo* ssi;
  public:
    /// Initialize
    PortfolioStop(Stop* so);
    /// Initialize shared stop information
    void share(SharedStopInfo* ssi);
    /// Return true if portfolio engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
  };

  /// Variant engine in a sequential portfolio
  class Variant {
  protected:
    /// The wrapped search engine
    Engine* engine;
    /// Stop object
    Stop* stop;
  public:
    /// Default constructor
    Variant(void);
    /// Copy constructor
    Variant(const Variant& s) = default;
    /// Assignment operator
    Variant& operator =(const Variant& s) = default;
    /// Initialize with engine \a s and its stop object \a so
    void init(Engine* s, Stop* so);
    /// Return next solution
    Space* next(void);
    /// Return statistics of the variant
    Statistics statistics(void) const;
    /// Check whether the variant has been stopped
    bool stopped(void) const;
    /// Constrain with better solution \a b
    void constrain(const Space& b);
    /// Perform one run
    void run(void);
    /// Delete variant
    ~Variant(void);
  };

  /// Sequential portfolio engine implementation
  template<bool best>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
  protected:
    /// Initial statistics
    Statistics stat;
    /// Shared variant information
    SharedStopInfo ssi;
    /// Size of a slice
    unsigned int slice;
    /// Variants
    Variant* variants;
    /// Number of variant engines
    unsigned int n_variants;
    /// Current variant to run
    unsigned int cur;
    /// Whether a variant has been stopped
    bool variant_stop;
  public:
    /// Initialize
    PBS(Engine** variants, Stop** stops, unsigned int n,
        const Statistics& stat, const Search::Options& opt);
    /// Return next solution (nullptr, if none exists or search has been stopped)
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
