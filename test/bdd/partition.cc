/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *
 *  Last modified:
 *     $Date: 2006-07-12 21:10:06 +0200 (Wed, 12 Jul 2006) $ by $Author: pekczynski $
 *     $Revision: 3350 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/bdd.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_33(-3,3);
static IntSet ds_4(4,4);
static IntSet ds_13(1,3);

class BddPartition : public BddTest {
public:
  BddPartition(const char* t) : BddTest(t,2,ds_13,false) {}
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::partition(home, x);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddPartition _bddpartition("Partition");


class BddPartitionLex : public BddTest {
public:
  BddPartitionLex(const char* t) : BddTest(t,2,ds_13,false) {}
//   template <class I>
//   int iter2int(I& r) const{
//     // std::cout << "iter2int\n";
//     // using ds_13 u = 3;
//     int u = 3;
//     if (!r()) {
//       return 0;
//     }
//     int v = 0;
//     // compute the bit representation of the assignment 
//     // and convert it to the corresponding integer
//     while(r()) {
//       v  |= ( 1 << (u - r.val()));
//       ++r;
//     }
//     return v;

//   }

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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::partitionLex(home, x, SRT_LE);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddPartitionLex _bddpartitionlex("Partition::Lex");


// STATISTICS: test-bdd
