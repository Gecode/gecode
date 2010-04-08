/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2008
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

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {

  /// %Tests for set variable conors
  namespace Var {

        /// %Test for cardinality constraint
    class Construct : public SetTest {
    public:
      /// Create and register test
      Construct(void)
        : SetTest("Var::Construct",1,IntSet(0,0),false,0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment&) const {
        return true;
      }
      void con(Space& home, int glbMin, int glbMax,
                     int lubMin, int lubMax,
                     unsigned int minCard = 0,
                     unsigned int maxCard = Gecode::Set::Limits::card,
                     bool fail=true) {
        bool ok = false;
        try {
          SetVar x(home, glbMin, glbMax, lubMin, lubMax, minCard, maxCard);
          ok = true;
        } catch (Gecode::Set::VariableEmptyDomain&) {
          if (!fail) {
            home.fail();
            return;
          }
          ok = true;
        }

        if (ok) {
          try {
            SetVarArray xs(home, 1,
                           glbMin, glbMax, lubMin, lubMax, minCard, maxCard);
          } catch (Gecode::Set::VariableEmptyDomain&) {
            if (!fail) {
              home.fail();
              return;
            } else {
              return;
            }
          }
          if (fail) {
            home.fail();
          }
        }
      }
      void con(Space& home, const IntSet& glb, int lubMin, int lubMax,
                     unsigned int minCard = 0,
                     unsigned int maxCard = Gecode::Set::Limits::card,
                     bool fail=true) {
        bool ok = false;
        try {
          SetVar x(home, glb, lubMin, lubMax, minCard, maxCard);
          ok = true;
        } catch (Gecode::Set::VariableEmptyDomain&) {
          if (!fail) {
            home.fail();
            return;
          }
          ok = true;
        }
        if (ok) {
          try {
            SetVarArray xs(home, 1,
                           glb, lubMin, lubMax, minCard, maxCard);
          } catch (Gecode::Set::VariableEmptyDomain&) {
            if (!fail) {
              home.fail();
              return;
            } else {
              return;
            }
          }
          if (fail) {
            home.fail();
          }
        }
      }
      void con(Space& home, int glbMin, int glbMax, const IntSet& lub,
                     unsigned int minCard = 0,
                     unsigned int maxCard = Gecode::Set::Limits::card,
                     bool fail=true) {
        bool ok = false;
        try {
          SetVar x(home, glbMin, glbMax, lub, minCard, maxCard);
          ok = true;
        } catch (Gecode::Set::VariableEmptyDomain&) {
          if (!fail) {
            home.fail();
            return;
          }
          ok = true;
        }
        if (ok) {
          try {
            SetVarArray xs(home, 1,
                           glbMin, glbMax, lub, minCard, maxCard);
          } catch (Gecode::Set::VariableEmptyDomain&) {
            if (!fail) {
              home.fail();
              return;
            } else {
              return;
            }
          }
          if (fail) {
            home.fail();
          }
        }
      }
      void con(Space& home, const IntSet& glb, const IntSet& lub,
                     unsigned int minCard = 0,
                     unsigned int maxCard = Gecode::Set::Limits::card,
                     bool fail=true) {
        bool ok = false;
        try {
          SetVar x(home, glb, lub, minCard, maxCard);
          ok = true;
        } catch (Gecode::Set::VariableEmptyDomain&) {
          if (!fail) {
            home.fail();
            return;
          }
          ok = true;
        }
        if (ok) {
          try {
            SetVarArray xs(home, 1,
                           glb, lub, minCard, maxCard);
          } catch (Gecode::Set::VariableEmptyDomain&) {
            if (!fail) {
              home.fail();
              return;
            } else {
              return;
            }
          }
          if (fail) {
            home.fail();
          }
        }
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray&, IntVarArray&) {
        // Check non-empty lower, empty upper bound
        con(home, 1,1, 1,0);
        con(home, IntSet(1,1), 1,0);
        con(home, 1,1, IntSet(1,0));
        con(home, IntSet(1,1), IntSet(1,0));

        // Check min cardinality
        con(home, 1,0, 1,1, 2,3);
        con(home, IntSet(1,0), 1,1, 2,3);
        con(home, 1,0, IntSet(1,1), 2,3);
        con(home, IntSet(1,0), IntSet(1,1), 2,3);

        // Check max cardinality
        con(home, 1,3, 1,4, 0,2);
        con(home, IntSet(1,3), 1,4, 0,2);
        con(home, 1,3, IntSet(1,4), 0,2);
        con(home, IntSet(1,3), IntSet(1,4), 0,2);

        // Check non-subset bounds
        con(home, 1,3, 2,4);
        con(home, IntSet(1,3), 2,4);
        con(home, 1,3, IntSet(2,4));
        con(home, IntSet(1,3), IntSet(2,4));
        con(home, 1,4, 1,3);
        con(home, IntSet(1,4), 1,3);
        con(home, 1,4, IntSet(1,3));
        con(home, IntSet(1,4), IntSet(1,3));
        
        con(home, IntSet::empty, 2, 4, 0, 10, false);
      }
    };
    Construct _con;


}}}

// STATISTICS: test-set
