/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Bugfixes provided by:
 *     Stefano Gualandi
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

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/path.hh>

namespace Gecode { namespace Search { namespace Sequential {

  /// Probe engine for %LDS
  class Probe : public Worker {
  protected:
    /// %Node in the search tree for %LDS
    class Node {
    private:
      /// %Space of current node
      Space*               _space;
      /// Branching description
      const BranchingDesc* _desc;
      /// Next alternative to try
      unsigned int         _alt;
    public:
      /// Default constructor
      Node(void);
      /// Initialize with node \a s, description \a d, and alternative \a a
      Node(Space* s, const BranchingDesc* d, unsigned int a);
      /// Return space
      Space* space(void) const;
      /// Return branching description
      const BranchingDesc* desc(void) const;
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
    /// Initialize for spaces of size \a s
    Probe(size_t s);
    /// Initialize with space \a s and discrepancy \a d
    void init(Space* s, unsigned int d);
    /// Reset with space \a s and discrepancy \a d
    void reset(Space* s, unsigned int d);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~Probe(void);
    /// %Search for next solution
    Space* explore(Stop* st);
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
    /// Initialize for space \a s (of size \a sz) with options \a o
    LDS(Space* s, size_t sz, const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
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
  Probe::Node::Node(Space* s, const BranchingDesc* d, unsigned int a)
    : _space(s), _desc(d), _alt(a) {}

  forceinline Space*
  Probe::Node::space(void) const {
    return _space;
  }

  forceinline const BranchingDesc*
  Probe::Node::desc(void) const {
    return _desc;
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
  Probe::Probe(size_t sz)
    : Worker(sz), ds(heap) {}

  forceinline void
  Probe::init(Space* s, unsigned int d0) {
    cur = s;
    d = d0;
    exhausted = true;
  }

  forceinline void
  Probe::reset(Space* s, unsigned int d0) {
    delete cur;
    assert(ds.empty());
    cur       = s;
    d         = d0;
    exhausted = true;
    Worker::reset(s);
  }

  forceinline Statistics
  Probe::statistics(void) const {
    Statistics s = *this;
    s.memory += ds.size();
    return s;
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
  Probe::explore(Stop* st) {
    start();
    while (true) {
      if (cur == NULL) {
      backtrack:
        if (ds.empty())
          return NULL;
        if (stop(st,ds.size()))
          return NULL;
        unsigned int a            = ds.top().alt();
        const BranchingDesc* desc = ds.top().desc();
        if (a == 0) {
          cur = ds.pop().space();
          Worker::pop(cur,desc);
          cur->commit(*desc,0);
          delete desc;
        } else {
          ds.top().next();
          cur = ds.top().space()->clone();
          cur->commit(*desc,a);
        }
        Worker::current(cur);
        d++;
      }
    check_discrepancy:
      if (d == 0) {
        Space* s = cur;
        while (s->status(*this) == SS_BRANCH) {
          const BranchingDesc* desc = s->description();
          if (desc->alternatives() > 1)
            exhausted = false;
          s->commit(*desc,0);
          delete desc;
        }
        cur = NULL;
        Worker::current(NULL);
        if (s->failed()) {
          delete s;
          goto backtrack;
        }
        // Deletes all pending branchings
        (void) s->description();
        return s;
      }
      node++;
      switch (cur->status(*this)) {
      case SS_FAILED:
        fail++;
      case SS_SOLVED:
        delete cur;
        cur = NULL;
        Worker::current(NULL);
        goto backtrack;
      case SS_BRANCH:
        {
          const BranchingDesc* desc = cur->description();
          unsigned int alt          = desc->alternatives();
          if (alt > 1) {
            if (d < alt-1)
              exhausted = false;
            unsigned int d_a = (d >= alt-1) ? alt-1 : d;
            Space* cc = cur->clone();
            Worker::push(cc,desc);
            Node sn(cc,desc,d_a-1);
            ds.push(sn);
            cur->commit(*desc,d_a);
            d -= d_a;
          } else {
            cur->commit(*desc,0);
            delete desc;
          }
          goto check_discrepancy;
        }
      default: GECODE_NEVER;
      }
    }
  }

  LDS::LDS(Space* s, size_t sz, const Options& o)
    : opt(o), e(sz), root(NULL), d(0) {
    if (s->status(e) == SS_FAILED) {
      e.init(NULL,0);
      e.fail += 1;
      e.current(s);
    } else {
      Space* c = snapshot(s,opt);
      if (opt.d > 0) {
        root = c->clone();
      }
      e.init(c,0);
      e.current(s);
      e.current(NULL);
      e.current(c);
    }
  }

  Space*
  LDS::next(void) {
    while (true) {
      Space* s = e.explore(opt.stop);
      if (s != NULL)
        return s;
      if (((s == NULL) && e.stopped()) || (++d > opt.d) || e.done())
        break;
      if (d == opt.d) {
        if (root != NULL)
          e.reset(root,d);
        root = NULL;
      } else if (root != NULL) {
        e.reset(root->clone(),d);
      }
    }
    return NULL;
  }

  bool
  LDS::stopped(void) const {
    return e.stopped();
  }

  Statistics
  LDS::statistics(void) const {
    return e.statistics();
  }

  LDS::~LDS(void) {
    delete root;
  }


  // Create lds engine
  Engine* lds(Space* s, size_t sz, const Options& o) {
    return new LDS(s,sz,o);
  }


}}}

// STATISTICS: search-any
