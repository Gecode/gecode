/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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


class BddPartitionNaryOne : public BddTest {
public:
  BddPartitionNaryOne(const char* t) : BddTest(t,3,ds_13,false, 0, 200, 10) {}
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    int n = x.size() - 1;
    BddVarArgs z(n);
    for (int i = 0; i < n; i++)
      z[i] = x[i];
    BddVar y = x[n];
    partition(home, z, y);
  }
};
BddPartitionNaryOne _bddpartitionnaryone("Partition::NaryOne");

class BddPartitionNaryOneSmaller : public BddTest {
public:
  BddPartitionNaryOneSmaller(const char* t) : BddTest(t,3,d1,false, 200, 80) {
    
  }
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    int n = x.size() - 1;
    BddVarArgs z(n);
    for (int i = 0; i < n; i++)
      z[i] = x[i];
    BddVar y = x[n];
    partition(home, z, y);
    dom(home, y, SRT_EQ, 3, 5);
  }
};
BddPartitionNaryOneSmaller _bddpartitionnaryonesmaller("Partition::NaryOne::Smaller");


class BddPartitionNaryOneInt : public BddTest {
public:
  BddPartitionNaryOneInt(const char* t) : BddTest(t,1,ds_13,false, 2, 200, 10) {}
  virtual bool solution(const SetAssignment& x) const {

    Assignment y = x.ints();

    Iter::Ranges::Singleton y0(y[0], y[0]);
    Iter::Ranges::Singleton y1(y[1], y[1]);
    Iter::Ranges::Union<Iter::Ranges::Singleton, Iter::Ranges::Singleton> u(y0, y1);

    CountableSetRanges xr(x.lub, x[0]);

    bool disjoint = y[0] != y[1];
    bool isunion  = Iter::Ranges::equal(u, xr);

    return disjoint && isunion;
  }

  virtual void post(Space* home, BddVarArray& x, IntVarArray& y) {
    BddVar z = x[0];
    std::cerr << "start of post\n";
    partition(home, y, z);
    std::cerr << "end of post\n";
  }
};

BddPartitionNaryOneInt _bddpartitionnaryoneint("Partition::NaryOne::Int");
// STATISTICS: test-bdd
