/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvis, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include "test/int.hh"
#include "test/log.hh"

static IntSet ds_14(1, 5);

class Table1 : public IntTest {
  ExtensionalAlgorithm ea;
public:
  Table1(const char* t, ExtensionalAlgorithm ea0)
    : IntTest(t,4,ds_14,false,ICL_DOM), ea(ea0) {}
  virtual bool solution(const Assignment& x) const {
    return (
            (x[0] == 1 && x[1] == 3 && x[2] == 2 && x[3] == 3) ||
            (x[0] == 2 && x[1] == 1 && x[2] == 2 && x[3] == 4) ||
            (x[0] == 2 && x[1] == 2 && x[2] == 1 && x[3] == 4) ||
            (x[0] == 3 && x[1] == 3 && x[2] == 3 && x[3] == 2) ||
            (x[0] == 4 && x[1] == 3 && x[2] == 4 && x[3] == 1)
            );
  }
  virtual void post(Space* home, IntVarArray& x) {
    Table t;
    IntArgs t1(4,  2, 1, 2, 4);
    IntArgs t2(4,  2, 2, 1, 4);
    IntArgs t3(4,  4, 3, 4, 1);
    IntArgs t4(4,  1, 3, 2, 3);
    IntArgs t5(4,  3, 3, 3, 2);
    t.add(t1);
    t.add(t2);
    t.add(t3);
    t.add(t4);
    t.add(t5);

    extensional(home, x, t, ea);
  }
};

namespace {
  Table1 _tab1b("Extensional::Table::Basic::1", EA_BASIC);
  Table1 _tab1i("Extensional::Table::Incremental::1", EA_INCREMENTAL);
}

class Table2 : public IntTest {
  ExtensionalAlgorithm ea;
  mutable Table t;
public:
  Table2(const char* name, ExtensionalAlgorithm ea0)
    : IntTest(name,4,ds_14,false,ICL_DOM), ea(ea0) {
    IntArgs t1 (4,  2, 1, 2, 4);
    IntArgs t2 (4,  2, 2, 1, 4);
    IntArgs t3 (4,  4, 3, 4, 1);
    IntArgs t4 (4,  1, 3, 2, 3);
    IntArgs t5 (4,  3, 3, 3, 2);
    IntArgs t6 (4,  5, 1, 4, 4);
    IntArgs t7 (4,  2, 5, 1, 5);
    IntArgs t8 (4,  4, 3, 5, 1);
    IntArgs t9 (4,  1, 5, 2, 5);
    IntArgs t10(4,  5, 3, 3, 2);
    t.add(t1);
    t.add(t2);
    t.add(t3);
    t.add(t4);
    t.add(t5);
    t.add(t6);
    t.add(t7);
    t.add(t8);
    t.add(t9);
    t.add(t10);
  }
  virtual bool solution(const Assignment& x) const {
    for (int i = 0; i < t.tuples(); ++i) {
      Table::tuple l = t[i];
      bool same = true;
      for (int j = 0; j < x.size() && same; ++j) {
        if (l[j] != x[j]) same = false;
      }
      if (same) return true;
    }
    return false;
  }
  virtual void post(Space* home, IntVarArray& x) {
    extensional(home, x, t, ea);
  }
};

namespace {
  Table2 _tab2b("Extensional::Table::Basic::2", EA_BASIC);
  Table2 _tab2i("Extensional::Table::Incremental::2", EA_INCREMENTAL);
}

// STATISTICS: test-int

