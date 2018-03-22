/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004, 2016
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

#ifndef __GECODE_SEARCH_SEQ_LDS_HH__
#define __GECODE_SEARCH_SEQ_LDS_HH__

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// %Probe engine for %LDS
  template<class Tracer>
  class Probe : public Worker {
  protected:
    /// Node identity type
    typedef typename Tracer::ID ID;
    /// %Node in the search tree for %LDS
    class Node {
    private:
      /// %Space of current node
      Space* _space;
      /// Choice
      const Choice* _choice;
      /// Next alternative to try
      unsigned int _alt;
      /// Node identifier
      ID _nid;
    public:
      /// Default constructor
      Node(void);
      /// Initialize with node \a s, choice \a c, and alternative \a a
      Node(Space* s, const Choice* c, unsigned int a, unsigned int nid);
      /// Return space
      Space* space(void) const;
      /// Return choice
      const Choice* choice(void) const;
      /// Return next alternative
      unsigned int alt(void) const;
      /// Return node identifier
      unsigned int nid(void) const;
      /// %Set next alternative
      void next(void);
    };
    /// Search tracer
    Tracer tracer;
    /// %Stack storing current path in search tree
    Support::DynamicStack<Node,Heap> ds;
    /// Current space
    Space* cur;
    /// Current discrepancy
    unsigned int d;
    /// Whether entire search space has been exhausted
    bool exhausted;
  public:
    /// Initialize
    Probe(const Options& opt);
    /// Initialize with space \a s
    void init(Space* s);
    /// Reset with space \a s and discrepancy \a d
    void reset(Space* s, unsigned int d);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~Probe(void);
    /// %Search for next solution
    Space* next(const Options& o);
    /// Test whether probing is done
    bool done(void) const;
  };

  /// Limited discrepancy search engine implementation
  template<class Tracer>
  class LDS : public Engine {
  protected:
    /// Search options
    Options opt;
    /// The probe engine
    Probe<Tracer> e;
    /// Root node for problem
    Space* root;
    /// Current discrepancy
    unsigned int d;
    /// Solution found for current discrepancy
    bool no_solution;
  public:
    /// Initialize for space \a s with options \a o
    LDS(Space* s, const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Constrain future solutions to be better than \a b (should never be called)
    void constrain(const Space& b);
    /// Reset engine to restart at space \a s
    void reset(Space* s);
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Destructor
    virtual ~LDS(void);
  };

}}}

#include <gecode/search/seq/lds.hpp>

#endif

// STATISTICS: search-seq
