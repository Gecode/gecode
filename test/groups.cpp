/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include "test/test.hh"

namespace Test { namespace Groups {

  /// Test space
  class GroupSpace : public Gecode::Space {
  protected:
    /// Integer variables
    Gecode::IntVarArray x;
  public:
    /// Propagator groups
    Gecode::PropagatorGroup a, b;
    /// Constructor for creation
    GroupSpace(int n) : x(*this,10,-10,10) {
      using namespace Gecode;
      switch (n) {
      case 2:
        distinct((*this)(b), x, IPL_DOM);
        // fall through
      case 1:
        rel((*this)(a), x[0], Gecode::IRT_LE, x[1]);
        break;
      default:
        break;
      }
    }
    /// Constructor for cloning \a s
    GroupSpace(GroupSpace& s) : Space(s), a(s.a), b(s.b) {
      x.update(*this,s.x);
    }
    /// Copy during cloning
    virtual Space* copy(void) {
      return new GroupSpace(*this);
    }
  };

  /// %Test for killing propagators
  class Kill : public Test::Base {
  protected:
    /// Number of propagators
    int n;
    /// Whether to compute fixpoint first
    bool fix;
    /// Whether to kill all propagators
    bool all;
  public:
    /// Initialize test
    Kill(int n0, bool fix0, bool all0)
      : Test::Base("PropagatorGroup::Kill::"+
                   str(n0)+"::"+str(fix0)+"::"+str(all0)),
        n(n0), fix(fix0), all(all0) {}
    /// Perform actual tests
    bool run(void) {
      using namespace Gecode;
      GroupSpace* s = new GroupSpace(n);
      if (fix)
        (void) s->status();
      if (all) {
        PropagatorGroup::all.kill(*s);
      } else {
        s->a.kill(*s);
        s->b.kill(*s);
      }
      (void) s->status();
      unsigned int p = PropagatorGroup::all.size(*s);
      delete s;
      return p == 0U;
    }
  };


  /// Help class to create and register tests
  class Create {
  public:
    /// Perform creation and registration
    Create(void) {
      for (int n=0; n<=2; n++) {
        (void) new Kill(n,true,false);
        (void) new Kill(n,false,false);
        (void) new Kill(n,true,true);
        (void) new Kill(n,false,true);
      }
    }
  };

  Create c;

}}

// STATISTICS: test-core
