/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#ifndef __GECODE_CPLTSETEXAMPLE_HH__
#define __GECODE_CPLTSETEXAMPLE_HH__

#include "gecode/cpltset.hh"
#include "examples/support.hh"

/// Extending %Gecode options for passing a manager for bdds 
class BddOptions : public Options {
public:
  BuddyMgr mgr;
  CpltSetOutput level;
  bool debug;
public:
  BddOptions(const char* s) 
    : Options(s), level(Gecode::CPLTSET_BND), debug(false){};
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
