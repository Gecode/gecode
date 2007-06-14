/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date: 2006-07-29 15:55:20 +0000 (Sat, 29 Jul 2006) $ by $Author: pekczynski $
 *     $Revision: 3478 $
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

#ifndef __GECODE_BDDEXAMPLE_HH__
#define __GECODE_BDDEXAMPLE_HH__

#include "gecode/bdd.hh"
#include "examples/support.hh"

/// Extending %Gecode options for passing a manager for bdds 
class BddOptions : public Options {
public:
  BuddyMgr mgr;
  BddOutput level;
  bool debug;
public:
  BddOptions(const char* s) 
    : Options(s), level(Gecode::BDD_BND), debug(false){};
};

/**
 * \brief Base-class for bdd examples
 *
 */
class BddExample : public Example {
public:
  BuddyMgr m;
  const BddOptions* bop;
  bool ordered;
  int  vn;
  int  cs;

  BddExample(void) {}
  BddExample(bool share, BddExample& e) 
    : Example(share,e), bop(e.bop), ordered(e.ordered), 
      vn(e.vn), cs(e.cs) {
    // update the manager handle
    m.update(this, share, e.m);
  }
  BddExample(const Options& op) {
    bop = reinterpret_cast<const BddOptions*> (&op);
    vn = bop->initvarnum;
    cs = bop->initcache;
    // initialize the manager with number of variable nodes vn
    // and cachesize for the bdd operators cs
    m.init(vn, cs);
    // set the output level from the option specified in the example
    m.level(bop->level);
    m.setmaxinc(5000);
    // check whether final output of used bdd nodes shall be printed
    if (bop->debug) {
      m.debug();
    }
  }
};

#endif

// STATISTICS: example-any
