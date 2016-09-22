/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004, 2016
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

#ifndef __GECODE_SEARCH_SEQUENTIAL_LDS_HH__
#define __GECODE_SEARCH_SEQUENTIAL_LDS_HH__

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>

namespace Gecode { namespace Search { namespace Sequential {

  /// %Probe engine for %LDS
  class Probe : public Worker {
  protected:
    /// %Node in the search tree for %LDS
    class Node {
    private:
      /// %Space of current node
      Space* _space;
      /// Choice
      const Choice* _choice;
      /// Next alternative to try
      unsigned int _alt;
    public:
      /// Default constructor
      Node(void);
      /// Initialize with node \a s, choice \a c, and alternative \a a
      Node(Space* s, const Choice* c, unsigned int a);
      /// Return space
      Space* space(void) const;
      /// Return choice
      const Choice* choice(void) const;
      /// Return next alternative
      unsigned int alt(void) const;
      /// %Set next alternative
      void next(void);
    };
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
    Probe(void);
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
  class LDS : public Engine {
  protected:
    /// Search options
    Options opt; 
    /// The probe engine
    Probe e;           
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

  /*
   * Nodes for the probing engine (just remember next alternative
   * to try)
   *
   */

  forceinline
  Probe::Node::Node(void) {}

  forceinline
  Probe::Node::Node(Space* s, const Choice* c, unsigned int a)
    : _space(s), _choice(c), _alt(a) {}

  forceinline Space*
  Probe::Node::space(void) const {
    return _space;
  }

  forceinline const Choice*
  Probe::Node::choice(void) const {
    return _choice;
  }

  forceinline unsigned int
  Probe::Node::alt(void) const {
    return _alt;
  }

  forceinline void
  Probe::Node::next(void) {
    _alt--;
  }


  /*
   * The probing engine: computes all solutions with
   * exact number of discrepancies (solutions with
   * fewer discrepancies are discarded)
   *
   */

  forceinline
  Probe::Probe(void)
    : ds(heap) {}

  forceinline void
  Probe::init(Space* s) {
    cur = s; d = 0U; exhausted = true;
  }

  forceinline void
  Probe::reset(Space* s, unsigned int d0) {
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
    cur = s; d = d0; exhausted = true;
    Worker::reset(0);
  }

  forceinline Statistics
  Probe::statistics(void) const {
    return *this;
  }

  forceinline bool
  Probe::done(void) const {
    return exhausted;
  }

  forceinline
  Probe::~Probe(void) {
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
  }

  forceinline Space*
  Probe::next(const Options& opt) {
    start();
    while (true) {
      if (cur == NULL) {
      backtrack:
        if (ds.empty())
          return NULL;
        if (stop(opt))
          return NULL;
        unsigned int a = ds.top().alt();
        const Choice* ch = ds.top().choice();
        if (a == 0) {
          cur = ds.pop().space();
          cur->commit(*ch,0);
          delete ch;
        } else {
          ds.top().next();
          cur = ds.top().space()->clone();
          cur->commit(*ch,a);
        }
        node++;
        d++;
      }
    check_discrepancy:
      if (d == 0) {
        Space* s = cur;
        while (s->status(*this) == SS_BRANCH) {
          if (stop(opt)) {
            cur = s;
            return NULL;
          }
          const Choice* ch = s->choice();
          if (ch->alternatives() > 1)
            exhausted = false;
          s->commit(*ch,0);
          node++;
          delete ch;
        }
        cur = NULL;
        if (s->failed()) {
          fail++;
          delete s;
          goto backtrack;
        }
        // Deletes all pending branchings
        (void) s->choice();
        return s;
      }
      node++;
      switch (cur->status(*this)) {
      case SS_FAILED:
        fail++;
      case SS_SOLVED:
        delete cur;
        cur = NULL;
        goto backtrack;
      case SS_BRANCH:
        {
          const Choice* ch = cur->choice();
          unsigned int alt = ch->alternatives();
          if (alt > 1) {
            if (d < alt-1)
              exhausted = false;
            unsigned int d_a = (d >= alt-1) ? alt-1 : d;
            Space* cc = cur->clone();
            Node sn(cc,ch,d_a-1);
            ds.push(sn);
            stack_depth(ds.entries());
            cur->commit(*ch,d_a);
            d -= d_a;
          } else {
            cur->commit(*ch,0);
            node++;
            delete ch;
          }
          goto check_discrepancy;
        }
      default: GECODE_NEVER;
      }
    }
  }

  forceinline
  LDS::LDS(Space* s, const Options& o)
    : opt(o), root(NULL), d(0) {
    e.node = 1;
    if (s->status(e) == SS_FAILED) {
      e.fail++;
      e.init(NULL);
    } else {
      Space* c = snapshot(s,opt);
      if (opt.d_l > 0) {
        root = c->clone();
      }
      e.init(c);
    }
  }

  forceinline void
  LDS::reset(Space* s) {
    delete root; root=NULL; d=0;
    e.node = 1;
    if ((s == NULL) || (s->status(e) == SS_FAILED)) {
      delete s;
      e.fail++;
      e.reset(NULL,0);
    } else {
      if (opt.d_l > 0) {
        root = s->clone();
      }
      e.reset(s,0);
    }
  }

  forceinline void
  LDS::constrain(const Space& b) {
    (void) b;
    assert(false);
  }

}}}

#endif

// STATISTICS: search-sequential
