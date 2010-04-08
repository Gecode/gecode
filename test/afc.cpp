/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

  /// %Test for %AFC infrastructure
  class AFC : public Test::Base {
  protected:
    /// Test space
    class TestSpace : public Gecode::Space {
    protected:
      /// Two integer variables
      Gecode::IntVar x, y;
    public:
      /// Constructor for creation
      TestSpace(void) : x(*this,0,10), y(*this,0,10) {}
      /// Constructor for cloning \a s
      TestSpace(bool share, TestSpace& s) : Space(share,s) {
        x.update(*this,share,s.x);
        y.update(*this,share,s.y);
      }
      /// Post arbitrary propagator
      void post(void) {
        Gecode::rel(*this, x, Gecode::IRT_LE, y);
      }
      /// Copy during cloning
      virtual Space* copy(bool share) {
        return new TestSpace(share,*this);
      }
    };
    /// How many test operations to be performed
    static const int n_ops = 8 * 1024;
    /// How many spaces to maintain
    static const int n = 16;
    /// Return random index of non-null space
    int space(TestSpace* s[]) {
      int i = rand(n);
      while (s[i] == NULL)
        i = (i+1) % n;
      return i;
    }
    /// Return random index
    int index(void) {
      return rand(n);
    }
  public:
    /// Initialize test
    AFC(void) : Test::Base("AFC") {}
    /// Perform actual tests
    bool run(void) {
      // Array of spaces for tests
      TestSpace* s[n];
      // How many spaces exist in s
      int n_s = 1;
      
      for (int i=n; i--; )
        s[i] = NULL;
      s[0] = new TestSpace;

      for (int o=n_ops; o--; )
        switch (rand(3)) {
        case 0:
          // clone space
          {
            int i = index();
            if ((s[i] != NULL)) {
              if (n_s > 1) {
                delete s[i]; s[i]=NULL; n_s--;
              } else {
                break;
              }
            }
            int j = space(s);
            (void) s[j]->status();
            s[i] = static_cast<TestSpace*>(s[j]->clone());
            n_s++;
          }
          break;
        case 1:
          // delete space
          if (n_s > 1) {
            int i = space(s);
            delete s[i]; s[i]=NULL; n_s--;
          }
          break;
        case 2:
          // post propagator
          s[space(s)]->post();
          break;
        default:
          GECODE_NEVER;
        }
      // Delete all remaining spaces
      for (int i=n; i--; )
        delete s[i];
      return true;
    }
  };

  AFC afc;

}

// STATISTICS: test-kernel
