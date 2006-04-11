/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date: 2005-11-07 17:15:40 +0100 (mån, 07 nov 2005) $ by $Author: zayenz $
 *     $Revision: 2519 $
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

#include "test/stress.hh"
#include "test/int.hh"
#include "test/log.hh"

#include "gecode/search.hh"

StressTestSpace::StressTestSpace(int n, IntSet dom, const Options& o) 
  : x(this, n, dom), opt(o) {
  Log::initial(x, "x");
}
StressTestSpace::StressTestSpace(bool share, StressTestSpace& s) 
  : Space(share,s), opt(s.opt) {
  x.update(this, share, s.x);
}
Space* StressTestSpace::copy(bool share) {
  return new StressTestSpace(share,*this);
}
bool StressTestSpace::is_failed(void) {
  unsigned int alt;
  Log::fixpoint();
  return status(alt) == SS_FAILED;
}

StressTestSpace *
StressTest::run(StressTestSpace *s) const {
  return dfs(s);
}

#define REPORT(M) \
problem = (M);    \
delete e;         \
goto failed;

bool 
StressTest::run(const Options& opt) {
  const char* problem = "NONE";
  while (param()) {
    Log::reset();
    int p = param.val();
    StressTestSpace *s = new StressTestSpace(vars(p),dom(p),opt);
    initialize(s,s->x,p);
    StressTestSpace *e = run(s);
    delete s;
    switch (check(e)) {
    case CR_OK:
      break;
    case CR_FOS:
      REPORT("Failed on solution");
      break;
    case CR_SOF:
      REPORT("Solved on failure");
      break;
    case CR_UNSPEC:
      REPORT("Unspecified error");
      break;
    }
    delete e;
    ++param;
  }
  return true;
 failed:
  std::cout << "FAILURE" << std::endl
	    << "\t" << "Problem:    " << problem << std::endl
	    << "\t" << "Parameter: " << param.val() << std::endl;
  return false;
}

#undef CHECK

// STATISTICS: test-stress

