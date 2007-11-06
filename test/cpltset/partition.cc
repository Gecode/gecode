/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include "test/cpltset.hh"

using namespace Gecode;
using namespace Test::Set;

namespace Test { namespace CpltSet {

  /// Tests for partition constraints
  namespace Partition {

    /**
      * \defgroup TaskTestCpltSetPartition Partition constraints
      * \ingroup TaskTestCpltSet
      */
    //@{

    static const int d1r[4][2] = {
      {-4,-3},{-1,-1},{1,1},{3,5}
    };
    static IntSet d1(d1r,4);

    static IntSet ds_33(-3,3);
    static IntSet ds_4(4,4);
    static IntSet ds_13(1,3);

    /// Test for partition constraint
    class CpltSetPartition : public CpltSetTest {
    public:
      /// Create and register test
      CpltSetPartition(const char* t) : CpltSetTest(t,2,ds_13,false) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr1(x.lub, x[0]);
        CountableSetRanges xr2(x.lub, x[1]);

        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
          d(xr1, xr2);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr1, xr2);
        IntSetRanges ir(ds_13);

        bool empty = !d();
        bool same  = Iter::Ranges::equal(u, ir);

        return empty && same;
      }

      /// Post constraint on \a x
      virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
        // Test lex-bit order
        Gecode::partition(home, x);
      }
    };
    CpltSetPartition _cpltsetpartition("Partition");

    /// Test for partition with lexicographic order constraint
    class CpltSetPartitionLex : public CpltSetTest {
    public:
      /// Create and register test
      CpltSetPartitionLex(const char* t) : CpltSetTest(t,2,ds_13,false) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr1(x.lub, x[0]);
        CountableSetRanges xr2(x.lub, x[1]);

        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
          d(xr1, xr2);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr1, xr2);
        IntSetRanges ir(ds_13);

        bool empty = !d();
        bool same  = Iter::Ranges::equal(u, ir);

        CountableSetValues xv1(x.lub, x[0]);
        CountableSetValues xv2(x.lub, x[1]);
        int a = iter2int(xv1, 3);
        int b = iter2int(xv2, 3);
    
        return empty && same && (a < b);
      }

      /// Post constraint on \a x
      virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
        Gecode::partitionLex(home, x, SRT_LE);
      }
    };
    CpltSetPartitionLex _cpltsetpartitionlex("Partition::Lex");

    /// Test for partition with result constraint
    class CpltSetPartitionNaryOne : public CpltSetTest {
    public:
      /// Create and register test
      CpltSetPartitionNaryOne(const char* t)
      : CpltSetTest(t,3,ds_13,false, 0, 200, 10) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr1(x.lub, x[0]);
        CountableSetRanges xr2(x.lub, x[1]);
        CountableSetRanges xr3(x.lub, x[2]);

        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
          d(xr1, xr2);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr1, xr2);

        bool empty = !d();
        bool same  = Iter::Ranges::equal(u, xr3);

        return empty && same;
      }

      /// Post constraint on \a x
      virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
        // Test lex-bit order
        int n = x.size() - 1;
        CpltSetVarArgs z(n);
        for (int i = 0; i < n; i++)
          z[i] = x[i];
        CpltSetVar y = x[n];
        partition(home, z, y);
      }
    };
    CpltSetPartitionNaryOne _cpltsetpartitionnaryone("Partition::NaryOne");

    /// Test for smaller instance of partition constraint
    class CpltSetPartitionNaryOneSmaller : public CpltSetTest {
    public:
      /// Create and register test
      CpltSetPartitionNaryOneSmaller(const char* t)
      : CpltSetTest(t,3,d1,false, 200, 80) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr1(x.lub, x[0]);
        CountableSetRanges xr2(x.lub, x[1]);
        CountableSetRanges xr3(x.lub, x[2]);
        Iter::Ranges::Cache<CountableSetRanges> cxr3(xr3);

        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
          d(xr1, xr2);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr1, xr2);

        bool empty = !d();
        bool same  = Iter::Ranges::equal(u, cxr3);

        cxr3.reset();
        Iter::Ranges::Singleton m(3,5);

        bool fixed = Iter::Ranges::equal(m, cxr3);

        return empty && same && fixed;
      }

      /// Post constraint on \a x
      virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
        int n = x.size() - 1;
        CpltSetVarArgs z(n);
        for (int i = 0; i < n; i++)
          z[i] = x[i];
        CpltSetVar y = x[n];
        partition(home, z, y);
        dom(home, y, SRT_EQ, 3, 5);
      }
    };
    CpltSetPartitionNaryOneSmaller 
      _cpltsetpartitionnaryonesmaller("Partition::NaryOne::Smaller");

    //@}

}}}

// STATISTICS: test-cpltset
