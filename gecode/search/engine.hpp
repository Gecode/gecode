/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

namespace Gecode { namespace Search {

  /**
   * \brief Probing engine for %LDS
   *
   */
  class ProbeEngine : public Engine {
  protected:
    /// %Node in the search tree for %LDS
    class ProbeNode {
    private:
      /// %Space of current node
      Space*               _space;
      /// Branching description
      const BranchingDesc* _desc;
      /// Next alternative to try
      unsigned int         _alt;
    public:
      /// Default constructor
      ProbeNode(void);
      /// Initialize with node \a s, description \a d, and alternative \a a
      ProbeNode(Space* s, const BranchingDesc* d, unsigned int a);
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
    Support::DynamicStack<ProbeNode,Heap> ds;
    /// Current space
    Space* cur;
    /// Current discrepancy
    unsigned int d;
    /// Whether entire search space has been exhausted
    bool exhausted;
  public:
    /// Initialize for spaces of size \a s
    ProbeEngine(size_t s);
    /// Initialize with space \a s and discrepancy \a d
    void init(Space* s, unsigned int d);
    /// Reset with space \a s and discrepancy \a d
    void reset(Space* s, unsigned int d);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~ProbeEngine(void);
    /// %Search for next solution
    Space* explore(Stop* st);
    /// Test whether probing is done
    bool done(void) const;
  };

}}

#include <gecode/search/engine/lds.hpp>

// STATISTICS: search-any
