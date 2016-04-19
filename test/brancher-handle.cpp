/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2013
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include "test/test.hh"

namespace Test {

  /// %Test for brancher handles
  class BrancherHandle : public Test::Base {
  protected:
    /// Test space
    class TestSpace : public Gecode::Space {
    protected:
      /// An array of integer variables
      Gecode::IntVarArray x;
    public:
      /// Constructor for creation
      TestSpace(void) : x(*this,2,0,10) {}
      /// Constructor for cloning \a s
      TestSpace(bool share, TestSpace& s) : Space(share,s) {
        x.update(*this,share,s.x);
      }
      /// Post arbitrary brancher and return its handle
      Gecode::BrancherHandle post(void) {
        using namespace Gecode;
        return Gecode::branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new TestSpace(share,*this);
      }
    };
    /// How many branchers to create
    static const int n_b = 1024;
    /// Return random index
    int index(void) {
      //      return rand(n);
      return 4;
    }
  public:
    /// Initialize test
    BrancherHandle(void) : Test::Base("BrancherHandle") {}
    /// Perform actual tests
    bool run(void) {
      using namespace Gecode;
      TestSpace* s = new TestSpace;

      // Create and immediately delete
      for (int i=0; i<n_b; i++) {
        Gecode::BrancherHandle b(s->post());
        if (!b(*s)) return false;
        b.kill(*s);
        if (b(*s)) return false;
      }

      if (s->status() != SS_SOLVED)
        return false;

      // Array of handles for tests
      Gecode::BrancherHandle bs[n_b];

      // Create and delete
      for (int i=0; i<n_b; i++) {
        Gecode::BrancherHandle b(s->post());
        if (!b(*s)) return false;
        bs[i] = b;
      }
      for (int i=0; i<n_b; i++) {
        bs[i].kill(*s);
        if (bs[i](*s)) return false;
      }
      if (s->status() != SS_SOLVED)
        return false;

      // Create and delete in inverse order
      for (int i=0; i<n_b; i++) {
        Gecode::BrancherHandle b(s->post());
        if (!b(*s)) return false;
        bs[i] = b;
      }
      for (int i=n_b; i--; ) {
        bs[i].kill(*s);
        if (bs[i](*s)) return false;
      }
      if (s->status() != SS_SOLVED)
        return false;

      // Create and delete randomly
      for (int i=0; i<n_b; i++) {
        Gecode::BrancherHandle b(s->post());
        if (!b(*s)) return false;
        bs[i] = b;
      }
      int a = n_b;
      while (a > 0) {
        int i = rand(n_b);
        if (bs[i](*s)) {
          bs[i].kill(*s); a--;
        }
      }
      if (s->status() != SS_SOLVED)
        return false;

      return true;
    }
  };

  BrancherHandle bh;

}

// STATISTICS: test-core
