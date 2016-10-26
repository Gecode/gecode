/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/search/path.hh
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_SEARCH_SEQUENTIAL_QDFS_HH__
#define __GECODE_SEARCH_SEQUENTIAL_QDFS_HH__

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>

#include <quacode/search/sequential/qpath.hh>

namespace Gecode { namespace Search { namespace Sequential {

  /// Depth-first search engine implementation
  class QDFS : public Worker {
  private:
    /// Search options
    Options opt;
    /// Current path ins search tree
    QPath path;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
  public:
    /// Initialize for space \a s with options \a o
    QDFS(Space* s, const Options& o);
    /// %Search for next solution
    Space* next(void);
    /// Return statistics
    Statistics statistics(void) const;
    /// Constrain future solutions to be better than \a b (should never be called)
    void constrain(const Space& b);
    /// Reset engine to restart at space \a s
    void reset(Space* s);
    /// Return no-goods
    NoGoods& nogoods(void);
    /// Destructor
    ~QDFS(void);
  };

  forceinline
  QDFS::QDFS(Space* s, const Options& o)
    : opt(o), path(static_cast<int>(opt.nogoods_limit)), d(0) {
    if ((s == NULL) || (s->status(*this) == SS_FAILED)) {
      fail++;
      cur = NULL;
      if (s) dynamic_cast<QSpaceInfo*>(s)->strategyFailed();
      if (!opt.clone)
        delete s;
    } else {
      cur = snapshot(s,opt);
      dynamic_cast<QSpaceInfo*>(cur)->strategyInit();
    }
  }

  forceinline void
  QDFS::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if ((s == NULL) || (s->status(*this) == SS_FAILED)) {
      cur = NULL;
      if (s) dynamic_cast<QSpaceInfo*>(s)->strategyFailed();
    } else {
      cur = s;
      dynamic_cast<QSpaceInfo*>(cur)->strategyReset();
    }
    Worker::reset();
  }

  forceinline NoGoods&
  QDFS::nogoods(void) {
    return path;
  }

  forceinline Space*
  QDFS::next(void) {
    Space * solvedSpace = NULL;
    Space * failedSpace = NULL;
    TQuantifier bckQuant = EXISTS;
    start();
    while (true) {
      while (cur) {
        if (stop(opt))
          return NULL;
        node++;

        SpaceStatus curStatus = cur->status(*this);
        // If no more propagators in space then all is succes below.
        // We deal with that case as any other success.
        // If it remains only Watch Propagators, then it is a success!
        if ((!cur->failed()) && (cur->propagators() == dynamic_cast<QSpaceInfo*>(cur)->watchConstraints())) curStatus = SS_SOLVED;

        switch (curStatus) {
        case SS_FAILED:
          dynamic_cast<QSpaceInfo*>(cur)->scenarioFailed();
          // On devra dépiler jusqu'au dernier existentiel
          bckQuant = EXISTS;
          fail++;
          failedSpace = cur;
          cur = NULL;
          break;
        case SS_SOLVED:
          dynamic_cast<QSpaceInfo*>(cur)->scenarioSuccess();
          // We will must go back to the last universal variable
          bckQuant = FORALL;
          // Deletes all pending branchers
          {
            const Choice* ch = cur->choice();
            if (ch) delete ch;
          }
          solvedSpace = cur;
          cur = NULL;
          break;
        case SS_BRANCH:
          {
            Space* c;
            if ((d == 0) || (d >= opt.c_d)) {
              c = cur->clone();
              d = 1;
            } else {
              c = NULL;
              d++;
            }
            const Choice* ch = path.push(*this,cur,c);
            cur->commit(*ch,0);
            cur->print(*ch,0,UNUSED_STREAM);
            break;
          }
        default:
          GECODE_NEVER;
        }
      }
      do {
        if (!path.next(bckQuant))
        {
          cur = NULL;
          if (solvedSpace)
          {
            dynamic_cast<QSpaceInfo*>(solvedSpace)->strategySuccess();
            return solvedSpace;
          } else {
            if (failedSpace)
            {
              // Can be NULL if failed space on construction
              dynamic_cast<QSpaceInfo*>(failedSpace)->strategyFailed();
              delete failedSpace;
            }
            return NULL;
          }
        }
        cur = path.recompute(d,opt.a_d,*this);
      } while (cur == NULL);
      if (solvedSpace)
      {
        delete solvedSpace;
        solvedSpace = NULL;
      }
      if (failedSpace)
      {
        delete failedSpace;
        failedSpace = NULL;
      }
    }
    GECODE_NEVER;
    return NULL;
  }

  forceinline Statistics
  QDFS::statistics(void) const {
    return *this;
  }

  forceinline void
  QDFS::constrain(const Space& b) {
    (void) b;
    assert(false);
  }

  forceinline
  QDFS::~QDFS(void) {
    delete cur;
    path.reset();
  }

}}}

#endif
