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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int.hh"
#include "test.hh"

using namespace Gecode;

class IntSetTest : public Test {
public:
  IntSetTest(void) : Test("IntSet","Test") {}
  bool run(const Options&)  {
    IntSet bi2(1,1);
    IntSet ci2;
    ci2.update(false, bi2);
    return true;
  }
};

IntSetTest _intset;

// STATISTICS: test-set
